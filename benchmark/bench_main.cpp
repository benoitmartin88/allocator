#include <benchmark/benchmark.h>

#include <vector>

#include "../src/memory.h"

using namespace root88::memory;



template <typename _T, template<typename> class _Allocator>
static void benchStdVector(benchmark::State& state) {
    static constexpr size_t SIZE = 100;

    for (auto _ : state) {
        auto v = std::vector<_T, _Allocator<_T>>();
        for(size_t i=0; i<SIZE; ++i) {
            v.emplace_back(i);
        }
    }
}



BENCHMARK_TEMPLATE(benchStdVector, size_t, std::allocator);
BENCHMARK_TEMPLATE(benchStdVector, size_t, LinearAllocator);
BENCHMARK_TEMPLATE(benchStdVector, size_t, PoolAllocator);

BENCHMARK_MAIN();
