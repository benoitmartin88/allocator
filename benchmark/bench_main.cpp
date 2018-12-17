#include <benchmark/benchmark.h>

#include <vector>

#include "../src/allocator.h"

using namespace root88::memory;



template <typename _T, template<typename> class _Allocator>
static void benchAllocateDeallocate(benchmark::State& state) {
    const auto size = static_cast<size_t>(state.range(0));
//    std::cout << ">>>>>>>>>>> size=" << size << std::endl;

    _Allocator<_T> allocator;
    _T* p = nullptr;

    for (auto _ : state) {
        benchmark::DoNotOptimize(p = allocator.allocate(size));
        allocator.deallocate(p, size);
    }
}



BENCHMARK_TEMPLATE(benchAllocateDeallocate, unsigned char, std::allocator)->RangeMultiplier(2)->Range(8, 8<<16);
//BENCHMARK_TEMPLATE(benchAllocateDeallocate, unsigned char, StaticBlockAllocator)->Range(8, 8<<13);
//BENCHMARK_TEMPLATE(benchAllocateDeallocate, unsigned char, ChainedBlockAllocator)->Range(8, 8<<13);
BENCHMARK_TEMPLATE(benchAllocateDeallocate, unsigned char, StaticChainedBlockAllocator)->RangeMultiplier(2)->Range(8, 8<<16);

//BENCHMARK_TEMPLATE(benchAllocateDeallocate, unsigned char, std::allocator)->Range(4096, 4096)->Iterations(1);
//BENCHMARK_TEMPLATE(benchAllocateDeallocate, unsigned char, StaticChainedBlockAllocator)->Range(4096, 4096)->Iterations(1);


BENCHMARK_MAIN();
