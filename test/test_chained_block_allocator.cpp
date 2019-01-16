#include <gtest/gtest.h>

#include <memory>
#include <new>

#include "../src/chained_block_allocator.h"


namespace root88 {
namespace memory {
namespace test {

    template<typename _T, template<typename> class _Allocator>
    void testCopyCtor() {
        static constexpr uint8_t SIZE = 8;

        _Allocator<_T> allocator1;
        auto ptr = allocator1.allocate(SIZE);

        for (uint8_t i = 0; i < SIZE; ++i) {
            _T *valPtr = new(ptr + i)(_T);
//        std::cout << "&valPtr=" << valPtr << std::endl;

            *valPtr = i;    // set pointer value

            ASSERT_EQ(ptr + i, valPtr);   // check pointer address
            ASSERT_EQ(i, *valPtr);      // check value
            ASSERT_EQ(i, *ptr + i);      // check value
        }



        // copy
        _Allocator<_T> allocator2 = allocator1;
        // TODO: check allocator internal memory using friend test class

//        std::cout << "test.allocator.indexFromBlockSize(SIZE)=" << test.allocator.indexFromBlockSize(SIZE) << std::endl;

//    for(uint8_t i=0; i<SIZE; ++i) {
//        ASSERT_EQ(ptr+i, test.allocator.blockListArray[test.allocator.indexFromBlockSize(SIZE)]);   // check pointer address
////        ASSERT_EQ(i, *intPtr);      // check value
////        ASSERT_EQ(i, *ptr+i);      // check value
//    }
    }

}   // namespace test
}   // namespace memory
}   // namespace root88


using namespace root88::memory;
using namespace root88::memory::test;


TEST(chained_block_allocator, copy_ctor) {
    testCopyCtor<unsigned char, ChainedBlockAllocator>();
}

// TODO test private methods: indexFromBlockSize, blockSizeFromIndex
