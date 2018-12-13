#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <new>

#include "../src/memory.h"

using namespace root88::memory;


template <typename _T, template<typename, std::size_t> class _Allocator>
void test_std_vector() {
    static constexpr uint8_t SIZE = 8;

    auto v = std::vector<_T, _Allocator<_T, SIZE>>(SIZE, 42);
    for(auto val : v) {
        ASSERT_EQ(42, val);
    }
}

template <typename _T, template<typename, std::size_t> class _Allocator>
void test_std_vector_bad_alloc() {
    static constexpr uint8_t SIZE = 8;

    auto v = std::vector<_T, _Allocator<_T, SIZE-1>>(SIZE, 42);
    for(auto val : v) {
        ASSERT_EQ(42, val);
    }
}



TEST(linear_allocator, std_vector) {
    test_std_vector<int, LinearAllocator>();
    test_std_vector<double, LinearAllocator>();
}

TEST(linear_allocator, bad_alloc) {
    bool caughtException = false;
    try {
        test_std_vector_bad_alloc<int, LinearAllocator>();
    } catch(const std::bad_alloc& e) {
        // expected
        std::cout << "Caught expected std::bad_alloc" << std::endl;
        caughtException = true;
    } catch(...) {
        FAIL();
    }
    ASSERT_TRUE(caughtException);
}

TEST(linear_allocator, new_placement) {
    static constexpr uint8_t SIZE = 8;

    root88::memory::LinearAllocator<int, SIZE> allocator;  // 8 * sizeof(int)
    auto ptr = allocator.allocate(SIZE);
    std::cout << "&ptr=" << ptr << std::endl;

    for(uint8_t i=0; i<SIZE; ++i) {
        int* intPtr = new(ptr+i)(int);
        std::cout << "&intPtr=" << intPtr << std::endl;

        *intPtr = i;    // set pointer

        ASSERT_EQ(ptr+i, intPtr);
        ASSERT_EQ(i, *intPtr);
    }

    int* intPtr = new(ptr)(int[SIZE]);
    for(uint8_t i=0; i<SIZE; ++i) {
        *intPtr = i;    // set pointer
        std::cout << "*intPtr=" << *intPtr << std::endl;
        ASSERT_EQ(ptr+i, intPtr+i);
        ASSERT_EQ(i, *intPtr);
    }
}


TEST(pool_allocator, std_vector) {
    test_std_vector<int, PoolAllocator>();
    test_std_vector<double, PoolAllocator>();
}

TEST(pool_allocator, bad_alloc) {
    try {
        test_std_vector_bad_alloc<int, PoolAllocator>();
    } catch(...) {
        FAIL();
    }
}