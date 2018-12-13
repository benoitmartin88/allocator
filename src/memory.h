//
// Created by MARTIN Benoît on 22/11/2018.
//

#ifndef MEMORY_MEMORY_H
#define MEMORY_MEMORY_H

#include <memory>
#include <stdlib.h>
#include <cassert>
#include <forward_list>
#include <iostream>     // TODO remove
#include <cmath>


namespace root88 {
namespace memory {
    static constexpr std::size_t DEFAULT_BLOCK_SIZE = 4096;


template <typename _T, std::size_t _SIZE=DEFAULT_BLOCK_SIZE>
class LinearAllocator {
public:
    typedef _T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _T* pointer;
    typedef const _T* const_pointer;
    typedef _T& reference;
    typedef const _T& const_reference;

    LinearAllocator() : ptr(new _T[_SIZE]), offset(ptr.get()) {
        std::cout << "LinearAllocator::LinearAllocator()" << std::endl;
    }

    LinearAllocator(const LinearAllocator& linearAllocator) : LinearAllocator() {
    }

    LinearAllocator(LinearAllocator&&) = delete;

    ~LinearAllocator() noexcept {
        ptr.reset(nullptr);
    };


    pointer allocate(size_type n, LinearAllocator<_T, _SIZE>::const_pointer hint=nullptr) {
        std::cout << "LinearAllocator::allocate(" << n << ", " << hint << ")" << std::endl;
        if(offset+n > ptr.get()+_SIZE) {
            throw std::bad_alloc();
        }

        _T* p = offset;
        offset += n;
        return p;
    }

    void deallocate(pointer p, size_type n) {
        std::cout << "LinearAllocator::deallocate(" << p << ", " << n << ")" << std::endl;
    }

//    template <class U>
//    struct rebind {
//        typedef LinearAllocator<U> other;
//    };
//
//    template <class U>
//    LinearAllocator(const LinearAllocator<U>&){
//    }
//
//    pointer address(reference x) const {return &x;}
//    const_pointer address(const_reference x) const {return &x;}
//    size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}
//
//    void construct(pointer p, const _T& val) {
//        new(static_cast<void*>(p)) _T(val);
//    }
//
//    void construct(pointer p) {
//        new(static_cast<void*>(p)) _T();
//    }
//
//    void destroy(pointer p) {
//        p->~_T();
//    }

private:
    // TODO: check bench with https://en.cppreference.com/w/cpp/types/aligned_storage
    std::unique_ptr<_T[]> ptr;
    _T* offset;
};


template <typename _T, std::size_t _BLOCK_SIZE=DEFAULT_BLOCK_SIZE>
class PoolAllocator {
public:
    typedef _T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _T* pointer;
    typedef const _T* const_pointer;
    typedef _T& reference;
    typedef const _T& const_reference;


    PoolAllocator(const uint64_t initialNumberOfBlocks=2)
            : memoryBlocks(initialNumberOfBlocks), currentBlockIt(memoryBlocks.before_begin()),
            currentBlockOffsetPtr(nullptr), initialNumberOfChunks(initialNumberOfBlocks) {
        std::cout << "PoolAllocator::PoolAllocator(" << initialNumberOfBlocks << ")" << std::endl;

        for(uint64_t i=0; i<initialNumberOfBlocks; ++i) {
            allocateNewBlock();
        }
        currentBlockIt = memoryBlocks.begin();
    }

    PoolAllocator(const PoolAllocator& poolAllocator) : PoolAllocator(poolAllocator.initialNumberOfChunks) {
    }

    PoolAllocator(PoolAllocator&&) = delete;

    ~PoolAllocator() noexcept {
        std::cout << "PoolAllocator::~PoolAllocator()" << std::endl;
        memoryBlocks.clear();
        currentBlockOffsetPtr = nullptr;
    };


    pointer allocate(size_type n, PoolAllocator<_T, _BLOCK_SIZE>::const_pointer hint=nullptr) {
        std::cout << "PoolAllocator::allocate(" << n << ", " << hint << ")" << std::endl;
        assert(currentBlockOffsetPtr != nullptr);
        assert(currentBlockOffsetPtr <= memoryBlocks.front().get());

        if(currentBlockOffsetPtr+n > currentBlockIt->get() + _BLOCK_SIZE) {
            // should not happen often. This will call new/malloc
            std::cout << "PoolAllocator::allocate(): n=" << n << std::endl;

            // current block is too small
            // either n is too big (n > _BLOCK_SIZE) -> allocate bigger block and use it
            // else next block is already allocated -> use it
            // or not -> allocate new block

            // try to use next block
            if(currentBlockIt != memoryBlocks.end() and n <= _BLOCK_SIZE) {
                std::cout << "PoolAllocator::allocate(): next block" << std::endl;
                ++currentBlockIt;   // next block
                currentBlockOffsetPtr = currentBlockIt->get();
            } else {
                // need a new block -> allocate
                std::size_t size = _BLOCK_SIZE; // default size
                if(n > _BLOCK_SIZE) {
                    // need a bigger block
                    size = _BLOCK_SIZE * static_cast<const std::size_t>(std::ceil(static_cast<float>(n)/_BLOCK_SIZE));
                }
                allocateNewBlock(size);
            }
        }

        _T* p = currentBlockOffsetPtr;
        currentBlockOffsetPtr += n;
        return p;
    }

    void deallocate(pointer p, size_type n) {
        std::cout << "PoolAllocator::deallocate(" << p << ", " << n << ")" << std::endl;
        // TODO
    }


private:
    // TODO: check bench with https://en.cppreference.com/w/cpp/types/aligned_storage
    std::forward_list<std::unique_ptr<_T[]>> memoryBlocks;
    typename std::forward_list<std::unique_ptr<_T[]>>::iterator currentBlockIt;
    _T* currentBlockOffsetPtr;
    const uint64_t initialNumberOfChunks;


    void allocateNewBlock(const std::size_t size=_BLOCK_SIZE) {
        assert(size%_BLOCK_SIZE==0);
        std::cout << "PoolAllocator::allocateNewBlock(" << size << ")" << std::endl;
        memoryBlocks.emplace_front(new _T[size]);    // allocate new block
        currentBlockOffsetPtr = memoryBlocks.front().get();
    }


};


template <typename T>
class stl_allocator {
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    stl_allocator(){}
    ~stl_allocator(){}

    template <class U> struct rebind { typedef stl_allocator<U> other; };
    template <class U> stl_allocator(const stl_allocator<U>&){}

    pointer address(reference x) const {return &x;}
    const_pointer address(const_reference x) const {return &x;}
    size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}

    pointer allocate(size_type n, stl_allocator<T>::const_pointer hint = 0) {
        return static_cast<pointer>(malloc(n*sizeof(T)));
    }

    void deallocate(pointer p, size_type n) {
        free(p);
    }

    void construct(pointer p, const T& val) {
        new(static_cast<void*>(p)) T(val);
    }

    void construct(pointer p) {
        new(static_cast<void*>(p)) T();
    }

    void destroy(pointer p) {
        p->~T();
    }
};


}
}

#endif //MEMORY_MEMORY_H
