#include <benchmark/benchmark.h>

#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>

// x86, none for ARM
// <intrin.h> on Windows
#include <emmintrin.h>

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
			// reschedule the current thread to reduce pressure on the lock
			std::this_thread::yield();
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
			/* on ARM:
			__asm__ __volatile__("wfe\n");
			*/
			// On x86:
			// emit a pause instruction.
			// this signals to the CPU that this thread is in a spin-loop
			// but does not release this thread to the OS scheduler!
			_mm_pause();
		}
	}

	void unlock() {
		/* on ARM:
		__asm__ __volatile__("sev\n");
		*/
		locked.store(false, std::memory_order_release);
	}
};

///////////////////// ✓ Locks /////////////////////

///////////////////// ● Helper methods /////////////////////
template <class TLock>
void UnlockNTimes(TLock& l, int n) {
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
		UnlockNTimes(*l, 1);
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
