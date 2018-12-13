#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <new>

#include "../src/memory.h"

using namespace root88::memory;


template <typename _T, template<typename> class _Allocator>
void test_std_vector() {
    static constexpr uint8_t SIZE = 8;

    auto v = std::vector<_T, _Allocator<_T>>(SIZE, 42);
    for(auto val : v) {
        ASSERT_EQ(42, val);
    }
}

template <typename _T, template<typename> class _Allocator>
static void test_std_vector_emplace_back() {
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
void test_std_vector_bad_alloc() {
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
 * LINEAR ALLOCATOR
 */
TEST(linear_allocator, std_vector) {
    test_std_vector<int, LinearAllocator>();
    test_std_vector<double, LinearAllocator>();
}

TEST(linear_allocator, std_vector_emplace_back) {
    test_std_vector_emplace_back<size_t, LinearAllocator>();
}

TEST(linear_allocator, bad_alloc) {
    test_std_vector_bad_alloc<int, LinearAllocator>();
}

TEST(linear_allocator, new_placement) {
    static constexpr uint8_t SIZE = 8;

    root88::memory::LinearAllocator<int> allocator;  // 8 * sizeof(int)
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

/*
 * POOL ALLOCATOR
 */
TEST(pool_allocator, std_vector) {
    test_std_vector<int, PoolAllocator>();
    test_std_vector<double, PoolAllocator>();
}

TEST(pool_allocator, std_vector_emplace_back) {
    test_std_vector_emplace_back<size_t, PoolAllocator>();
}

