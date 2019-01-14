#include <benchmark/benchmark.h>

#include <vector>
#include <chrono>

#include "../src/chained_block_allocator.h"

using namespace root88::memory;



void computeStats(benchmark::State &state, std::vector<double>& data) {
    assert(data.size() == state.iterations());

    std::sort(data.begin(), data.end());
    state.counters["min"] =  benchmark::Counter(data[0], benchmark::Counter::kAvgThreads);
    state.counters["max"] = benchmark::Counter(data[data.size()-1], benchmark::Counter::kAvgThreads);

    state.counters["50"] = benchmark::Counter(data[0.50*data.size()], benchmark::Counter::kAvgThreads);
    state.counters["75"] = benchmark::Counter(data[0.75*data.size()], benchmark::Counter::kAvgThreads);
    state.counters["99"] = benchmark::Counter(data[0.99*data.size()], benchmark::Counter::kAvgThreads);
    state.counters["99.9"] = benchmark::Counter(data[0.999*data.size()], benchmark::Counter::kAvgThreads);
    state.counters["99.99"] = benchmark::Counter(data[0.9999*data.size()], benchmark::Counter::kAvgThreads);
//    state.counters["99.999"] = benchmark::Counter(data[0.99999*data.size()], benchmark::Counter::kAvgThreads);
//    state.counters["99.9999"] = benchmark::Counter(data[0.999999*data.size()], benchmark::Counter::kAvgThreads);

    data.clear();   // clear map
}


template <typename _T, template<typename> class _Allocator>
static void benchAllocate(benchmark::State &state) {
    const auto size = static_cast<size_t>(state.range(0));
//    std::cout << ">>>>>>>>>>> size=" << size << std::endl;

    _Allocator<_T> allocator;
    _T* p = nullptr;
    std::vector<double> data;

    for(auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(p = allocator.allocate(size));

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed.count());
        data.push_back(elapsed.count());

        allocator.deallocate(p, size);
    }

    computeStats(state, data);
}

template <typename _T, template<typename> class _Allocator>
static void benchDeallocate(benchmark::State &state) {
    const auto size = static_cast<size_t>(state.range(0));
//    std::cout << ">>>>>>>>>>> size=" << size << std::endl;

    _Allocator<_T> allocator;
    _T* p = nullptr;
    std::vector<double> data;

    for(auto _ : state) {
        benchmark::DoNotOptimize(p = allocator.allocate(size));

        auto start = std::chrono::high_resolution_clock::now();

        allocator.deallocate(p, size);

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        state.SetIterationTime(elapsed.count());
        data.push_back(elapsed.count());
    }

    computeStats(state, data);
}



BENCHMARK_TEMPLATE(benchAllocate, unsigned char, std::allocator)->RangeMultiplier(2)->Range(8, 8<<13)->UseManualTime()->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(benchAllocate, unsigned char, ChainedBlockAllocator)->RangeMultiplier(2)->Range(8, 8<<13)->UseManualTime()->Unit(benchmark::kMillisecond);

//BENCHMARK_TEMPLATE(benchDeallocate, unsigned char, std::allocator)->RangeMultiplier(2)->Range(8, 8<<13)->UseManualTime()->Unit(benchmark::kMillisecond)->ThreadRange(1, 16);
//BENCHMARK_TEMPLATE(benchDeallocate, unsigned char, StaticChainedBlockAllocator)->RangeMultiplier(2)->Range(8, 8<<13)->UseManualTime()->Unit(benchmark::kMillisecond)->ThreadRange(1, 16);


//static constexpr int SIZE = 30;
//
//BENCHMARK_TEMPLATE(benchAllocate, unsigned char, std::allocator)
//        ->Arg(SIZE)
//        ->UseManualTime()->Unit(benchmark::kMillisecond);
//
//BENCHMARK_TEMPLATE(benchAllocate, unsigned char, StaticChainedBlockAllocator)
//        ->Arg(SIZE)
//        ->UseManualTime()->Unit(benchmark::kMillisecond);
//
//BENCHMARK_TEMPLATE(benchDeallocate, unsigned char, std::allocator)
//        ->Arg(SIZE)
//        ->UseManualTime()->Unit(benchmark::kMillisecond);
//
//BENCHMARK_TEMPLATE(benchDeallocate, unsigned char, StaticChainedBlockAllocator)
//        ->Arg(SIZE)
//        ->UseManualTime()->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
