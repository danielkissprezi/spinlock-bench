#include <benchmark/benchmark.h>

#include <cstdint>
#include <atomic>
#include <thread>
#include <mutex>
#include <algorithm>

#if defined(__amd64__) || defined(_M_X64)
#define SPIN_X64 1

#elif defined(__ARM_ARCH)

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
				asm volatile("wfe");
#endif
#if SPIN_X64
				// emit a pause instruction.
				// this signals to the CPU that this thread is in a spin-loop
				// but does not release this thread to the OS scheduler!
				asm volatile("pause");
#endif
			} while (locked.load(std::memory_order_relaxed));
		}
	}

	void unlock() {
		locked.store(false, std::memory_order_release);
#if SPIN_ARM
		asm volatile("sev");
#endif
	}
};
// #if SPIN_ARM
// struct Cursed {
// 	std::atomic<int> locked{0};

// 	void lock() {
// 		int lockval, flagval;
// 		const int tmp = 1;
// 		for (;;) {
// 			/*
// 			Exchange the `locked` value
// 			If the original value was 1: jump to the read loop
// 			If the exchange fails try again
// 			If we aquired the lock (exchange succeeded and the original value was 0): return
// 			*/
// 			asm volatile(
// 				R"(
// 1:.try_read:            
//     ldaxrb %w0, %2
//     stxrb %w1, %w3, %2
//     cbnz %w1, 1b
//     cbnz %w0, 1f
//     ret
// 1:.read_loop:
// )"
// 				: "=r"(lockval), "=r"(flagval), "=m"(locked)
// 				: "r"(tmp));

// 			do {
// 				asm volatile("wfe");
// 			} while (locked.load(std::memory_order_relaxed));
// 		}
// 	}

// 	void unlock() {
// 		locked.store(0, std::memory_order_release);
// 		asm volatile("sev");
// 	}
// };
// #endif

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

#define LOCK_BENCH_IMPL(method, lock, n) \
	BENCHMARK_TEMPLATE(method, lock)->Threads(n)->UseRealTime();

#define LOCK_BENCH(method, lock)      \
	LOCK_BENCH_IMPL(method, lock, 1); \
	LOCK_BENCH_IMPL(method, lock, 2); \
	LOCK_BENCH_IMPL(method, lock, 4); \
	LOCK_BENCH_IMPL(method, lock, 8); \
	LOCK_BENCH_IMPL(method, lock, 16);

LOCK_BENCH(HeavyContention, NoYield);
LOCK_BENCH(HeavyContention, Pause);
LOCK_BENCH(HeavyContention, Yield);
LOCK_BENCH(HeavyContention, std::mutex);
#if SPIN_ARM
LOCK_BENCH(HeavyContention, Cursed);
#endif

BENCHMARK_MAIN();
