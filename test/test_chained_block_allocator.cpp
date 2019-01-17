#include <gtest/gtest.h>

#include <memory>
#include <new>

#include "../src/chained_block_allocator.h"


namespace root88 {
namespace memory {

    template<typename _T>
    class ChainedBlockAllocator<_T>::Test {
    public:
        Test(ChainedBlockAllocator<_T>& allocator) : allocator(allocator) {
        }

        void testIndexFromBlockSize() {
            ASSERT_EQ(0, allocator.indexFromBlockSize(1));

            ASSERT_EQ(1, allocator.indexFromBlockSize(2));
            ASSERT_EQ(1, allocator.indexFromBlockSize(3));

            ASSERT_EQ(2, allocator.indexFromBlockSize(4));
            ASSERT_EQ(2, allocator.indexFromBlockSize(5));
            ASSERT_EQ(2, allocator.indexFromBlockSize(6));
            ASSERT_EQ(2, allocator.indexFromBlockSize(7));

            ASSERT_EQ(3, allocator.indexFromBlockSize(8));
            ASSERT_EQ(3, allocator.indexFromBlockSize(9));
            ASSERT_EQ(3, allocator.indexFromBlockSize(10));
            ASSERT_EQ(3, allocator.indexFromBlockSize(11));
            ASSERT_EQ(3, allocator.indexFromBlockSize(12));
            ASSERT_EQ(3, allocator.indexFromBlockSize(13));
            ASSERT_EQ(3, allocator.indexFromBlockSize(14));
            ASSERT_EQ(3, allocator.indexFromBlockSize(15));


            for(size_t i=4; i<16; ++i) {
                for(size_t j=(size_t(1)<<i); j<(size_t(1)<<(i+1)); ++j) {
                    ASSERT_EQ(i, allocator.indexFromBlockSize(j));
                }
            }

            // Compute intensive, skip from 17 to 62.

            size_t k = size_t(1)<<63;
            ASSERT_EQ(63, allocator.indexFromBlockSize(k*2-1));     // last element
        }

        void testBlockSizeFromIndex() {

            ASSERT_EQ(1, allocator.blockSizeFromIndex(0));
            ASSERT_EQ(2, allocator.blockSizeFromIndex(1));
            ASSERT_EQ(4, allocator.blockSizeFromIndex(2));

            for(ChainedBlockAllocator<_T>::blockListIndex_t i=0; i<allocator.BLOCK_LIST_SIZE; ++i) {
                ASSERT_EQ(size_t(1) << i, allocator.blockSizeFromIndex(i));
            }

            ASSERT_EQ(9223372036854775808u, allocator.blockSizeFromIndex(63));  // last element
        }

        void testEquality(const ChainedBlockAllocator<_T>& other) {
            for(ChainedBlockAllocator<_T>::blockListIndex_t i=0; i<ChainedBlockAllocator<_T>::BLOCK_LIST_SIZE; ++i) {
                ASSERT_TRUE(std::distance(allocator.blockListArray[i].begin(), allocator.blockListArray[i].end())
                    == std::distance(other.blockListArray[i].begin(), other.blockListArray[i].end()));
            }
        }

    private:
        ChainedBlockAllocator<_T>& allocator;
    };

    template<typename _T>
    using ChainedBlockAllocatorTest = typename ChainedBlockAllocator<_T>::Test;

namespace test {

}   // namespace test
}   // namespace memory
}   // namespace root88


using namespace root88::memory;
using namespace root88::memory::test;


TEST(chained_block_allocator, private_methods) {
    ChainedBlockAllocator<char> allocator;
    ChainedBlockAllocatorTest<char> test(allocator);

    // allocate arbitrary size
    for(size_t s=1; s<12345; ++s) {
        auto p = allocator.allocate(s);
        allocator.deallocate(p, s);
    }

    // copy allocator
    ChainedBlockAllocator<char> allocatorCpy(allocator);

    test.testIndexFromBlockSize();
    test.testBlockSizeFromIndex();
    test.testEquality(allocatorCpy);
}

// TODO test private methods: indexFromBlockSize, blockSizeFromIndex
