#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <new>

#include "../src/allocator.h"

using namespace root88::memory;


template <typename _T, template<typename> class _Allocator>
void testStdVector() {
    static constexpr uint8_t SIZE = 8;

    auto v = std::vector<_T, _Allocator<_T>>(SIZE, 42);
    for(auto val : v) {
        ASSERT_EQ(42, val);
    }
}

template <typename _T, template<typename> class _Allocator>
void testStdVectorEmplace(const _T val) {
    static constexpr uint8_t SIZE = 8;

    auto v = std::vector<_T, _Allocator<_T>>(SIZE);
    for(size_t i=0; i<SIZE; ++i) {
        std::cout << "emplace_back i=" << i << std::endl;
        v.emplace_back(val);
        ASSERT_EQ(val, v.back());
    }
}

template <typename _T, template<typename> class _Allocator>
static void testStdVectorEmplaceBack() {
    static constexpr size_t SIZE = 100;

    auto v = std::vector<_T, _Allocator<_T>>();
    for(size_t i=0; i<SIZE; ++i) {
        v.emplace_back(i);
        ASSERT_EQ(i, v.back());
    }

    for(size_t i=0; i<SIZE; ++i) {
        ASSERT_EQ(i, v.at(i));
    }

    ASSERT_EQ(SIZE, v.size());
}

template <typename _T, template<typename> class _Allocator>
void testStdVectorBadAlloc() {
    static constexpr std::size_t SIZE = 8;

    bool caughtException = false;
    try {
        _Allocator<_T> allocator(SIZE-1);
        auto v = std::vector<_T, _Allocator<_T>>(SIZE, 42, allocator);
        for(auto val : v) {
            ASSERT_EQ(42, val);
        }
    } catch(const std::bad_alloc& e) {
        // expected
        std::cout << "Caught expected std::bad_alloc" << std::endl;
        caughtException = true;
    } catch(...) {
        FAIL();
    }
    ASSERT_TRUE(caughtException);
}


/*
 * STATIC BLOCK ALLOCATOR
 */
TEST(static_block_allocator, std_vector) {
    testStdVector<int, StaticBlockAllocator>();
    testStdVector<double, StaticBlockAllocator>();
}

TEST(static_block_allocator, std_vector_emplace_back) {
    testStdVectorEmplaceBack<size_t, StaticBlockAllocator>();
}

TEST(static_block_allocator, bad_alloc) {
    testStdVectorBadAlloc<int, StaticBlockAllocator>();
}

TEST(static_block_allocator, new_placement) {
    static constexpr uint8_t SIZE = 8;

    root88::memory::StaticBlockAllocator<int> allocator;  // 8 * sizeof(int)
    auto ptr = allocator.allocate(SIZE);
//    std::cout << "&ptr=" << ptr << std::endl;

    for(uint8_t i=0; i<SIZE; ++i) {
        int* intPtr = new(ptr+i)(int);
//        std::cout << "&intPtr=" << intPtr << std::endl;

        *intPtr = i;    // set pointer value

        ASSERT_EQ(ptr+i, intPtr);   // check pointer address
        ASSERT_EQ(i, *intPtr);      // check value
        ASSERT_EQ(i, *ptr+i);      // check value
    }

    int* intPtr = new(ptr)(int[SIZE]);
    for(uint8_t i=0; i<SIZE; ++i) {
        *(intPtr+i) = i;    // set pointer value
//        std::cout << "*intPtr=" << *(intPtr+i) << std::endl;
        ASSERT_EQ(ptr+i, intPtr+i);     // check pointer address
        ASSERT_EQ(i, *(intPtr+i));      // check value
    }

    allocator.deallocate(ptr, SIZE);    // explicit call to deallocate due to new placement
}

/*
 * CHAINED BLOCK ALLOCATOR
 */
TEST(pool_allocator, std_vector) {
    testStdVector<int, ChainedBlockAllocator>();
    testStdVector<double, ChainedBlockAllocator>();
}

TEST(pool_allocator, std_vector_emplace_back) {
    testStdVectorEmplaceBack<size_t, ChainedBlockAllocator>();
}

/*
 * STATIC CHAINED BLOCK ALLOCATOR
 */
TEST(staticChainedBlockAllocator, std_vector) {
//    testStdVector<int, StaticChainedBlockAllocator>();
//    testStdVector<double, StaticChainedBlockAllocator>();
    testStdVectorEmplace<unsigned char, StaticChainedBlockAllocator>('a');
}
