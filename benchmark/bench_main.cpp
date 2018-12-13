#include <benchmark/benchmark.h>

#include <vector>

#include "../src/allocator.h"

using namespace root88::memory;



template <typename _T, template<typename> class _Allocator>
static void benchStdVector(benchmark::State& state) {
    const auto size = static_cast<size_t>(state.range(0));
//    std::cout << ">>>>>>>>>>> size=" << size << std::endl;

    for (auto _ : state) {
        state.PauseTiming();
        auto v = std::vector<_T, _Allocator<_T>>();
        state.ResumeTiming();

        for(size_t i=0; i<size; ++i) {
//            std::cout << "emplace_back i=" << i << std::endl;
            v.emplace_back('a');
        }
    }
}



BENCHMARK_TEMPLATE(benchStdVector, unsigned char, std::allocator)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(benchStdVector, unsigned char, StaticBlockAllocator)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(benchStdVector, unsigned char, ChainedBlockAllocator)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(benchStdVector, unsigned char, StaticChainedBlockAllocator)->Range(8, 8<<10);

//BENCHMARK_TEMPLATE(benchStdVector, size_t, std::allocator)->Range(2, 2)->Iterations(1);
//BENCHMARK_TEMPLATE(benchStdVector, size_t, StaticBlockAllocator)->Range(2, 2)->Iterations(1);


BENCHMARK_MAIN();
