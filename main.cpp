#include <benchmark/benchmark.h>

#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>

#if defined(__amd64__) || defined(_M_X64)
#define SPIN_X64 1

#if defined(_MSVC_VER)
// thx msvc
#include <inrin.h>
#else
#include <emmintrin.h>
#endif

#elif defined(__arm__)

#define SPIN_ARM 1


#else

#error TODO

#endif

///////////////////// ● Locks /////////////////////
struct NoYield {
	std::atomic<bool> locked{false};

	void lock() {
		while (locked.exchange(true, std::memory_order_acquire))
			;  // busy wait
	}

	void unlock() {
		locked.store(false, std::memory_order_release);
	}
};

struct Yield {
	std::atomic<bool> locked{false};

	void lock() {
		while (locked.exchange(true, std::memory_order_acquire)) {
			do {
				// reschedule the current thread to reduce pressure on the lock
				std::this_thread::yield();
			} while (locked.load(std::memory_order_relaxed));
		}
	}

	void unlock() {
		locked.store(false, std::memory_order_release);
	}
};

struct Pause {
	std::atomic<bool> locked{false};

	void lock() {
		while (locked.exchange(true, std::memory_order_acquire)) {	// retry loop
			do {
#if SPIN_ARM
				asm volatile("wfe\n");
#endif
#if SPIN_X64
				// emit a pause instruction.
				// this signals to the CPU that this thread is in a spin-loop
				// but does not release this thread to the OS scheduler!
				_mm_pause();
#endif
			} while (locked.load(std::memory_order_relaxed));
		}
	}

	void unlock() {
		locked.store(false, std::memory_order_release);
#if SPIN_ARM
		asm volatile("sev\n");
#endif
	}
};

///////////////////// ✓ Locks /////////////////////

///////////////////// ● Helper methods /////////////////////
template <int n, class TLock>
void UnlockNTimes(TLock& l) {
	for (int i = 0; i < n; ++i) {
		l.lock();
		// force lock write to global memory
		//*
		benchmark::DoNotOptimize(l);
		benchmark::ClobberMemory();
		//*/
		l.unlock();
	}
}

///////////////////// ✓ Helper methods /////////////////////

template <class TLock>
void HeavyContention(benchmark::State& state) {
	static TLock* l;
	if (state.thread_index() == 0) {
		l = new TLock();
	}
	for (auto _ : state) {
		UnlockNTimes<1>(*l);
	}
	// cleanup
	if (state.thread_index() == 0) {
		delete l;
	}
}

#define LOCK_BENCH(method, lock)                                 \
	BENCHMARK_TEMPLATE(method, lock)->Threads(1)->UseRealTime(); \
	BENCHMARK_TEMPLATE(method, lock)->Threads(2)->UseRealTime(); \
	BENCHMARK_TEMPLATE(method, lock)->Threads(4)->UseRealTime(); \
	BENCHMARK_TEMPLATE(method, lock)->Threads(8)->UseRealTime(); \
	BENCHMARK_TEMPLATE(method, lock)->Threads(16)->UseRealTime();

LOCK_BENCH(HeavyContention, NoYield)
LOCK_BENCH(HeavyContention, Yield)
LOCK_BENCH(HeavyContention, Pause)
LOCK_BENCH(HeavyContention, std::mutex)

BENCHMARK_MAIN();
