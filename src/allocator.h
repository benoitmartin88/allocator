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
    static constexpr std::size_t DEFAULT_BLOCK_SIZE = 4096 * 1024;  // 4096 * 1024 = 4194304 * sizeof(_T)


template <typename _T>
class StaticBlockAllocator {
public:
    typedef _T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _T* pointer;
    typedef const _T* const_pointer;
    typedef _T& reference;
    typedef const _T& const_reference;

    StaticBlockAllocator(const std::size_t size=DEFAULT_BLOCK_SIZE) : size(size), ptr(new _T[size]), offset(ptr.get())
#ifndef NDEBUG
            , debugAllocatedSize(0)
#endif
    {

    }

    StaticBlockAllocator(const StaticBlockAllocator& linearAllocator) : StaticBlockAllocator(linearAllocator.size) {
    }

    StaticBlockAllocator(StaticBlockAllocator&&) = delete;

    ~StaticBlockAllocator() noexcept {
#ifndef NDEBUG
        std::cout << "StaticBlockAllocator::~StaticBlockAllocator(): debugAllocatedSize=" << debugAllocatedSize << std::endl;
#endif
        ptr.reset();
        assert(debugAllocatedSize==0);    // TODO
    };


    pointer allocate(size_type n, StaticBlockAllocator<_T>::const_pointer hint=nullptr) {
#ifndef NDEBUG
        std::cout << "StaticBlockAllocator::allocate(n=" << n << ")" << std::endl;
#endif
        if(offset+n > ptr.get()+size) {
            throw std::bad_alloc();
        }

        _T* p = offset;
        offset += n;
#ifndef NDEBUG
        debugAllocatedSize += n;
#endif
        return p;
    }

    void deallocate(pointer p, size_type n) {
#ifndef NDEBUG
        debugAllocatedSize -= n;
        std::cout << "StaticBlockAllocator::deallocate(p=" << p << ", n=" << n << "): debugAllocatedSize=" << debugAllocatedSize << std::endl;
#endif
    }

//    template <class U>
//    struct rebind {
//        typedef StaticBlockAllocator<U> other;
//    };
//
//    template <class U>
//    StaticBlockAllocator(const StaticBlockAllocator<U>&){
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
    const std::size_t size;
    std::unique_ptr<_T[]> ptr;
    _T* offset;
#ifndef NDEBUG
    uint64_t debugAllocatedSize;
#endif
};


template <typename _T>
class ChainedBlockAllocator {
public:
    typedef _T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _T* pointer;
    typedef const _T* const_pointer;
    typedef _T& reference;
    typedef const _T& const_reference;


    ChainedBlockAllocator(const uint64_t initialNumberOfBlocks=2, const std::size_t blockSize=DEFAULT_BLOCK_SIZE)
            : initialNumberOfChunks(initialNumberOfBlocks), blockSize(blockSize),
            memoryBlocks(initialNumberOfBlocks), currentBlockIt(memoryBlocks.before_begin()),
            currentBlockOffsetPtr(nullptr) {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::ChainedBlockAllocator(initialNumberOfBlocks=" << initialNumberOfBlocks << ")" << std::endl;
#endif

        for(uint64_t i=0; i<initialNumberOfBlocks; ++i) {
            allocateNewBlock();
        }
        currentBlockIt = memoryBlocks.begin();
    }

    ChainedBlockAllocator(const ChainedBlockAllocator& poolAllocator) : ChainedBlockAllocator(poolAllocator.initialNumberOfChunks, poolAllocator.blockSize) {
    }

    ChainedBlockAllocator(ChainedBlockAllocator&&) = delete;

    ~ChainedBlockAllocator() noexcept {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::~ChainedBlockAllocator()" << std::endl;
#endif
        memoryBlocks.clear();
        currentBlockOffsetPtr = nullptr;
    };


    pointer allocate(size_type n, ChainedBlockAllocator<_T>::const_pointer hint=nullptr) {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::allocate(" << n << ")" << std::endl;
#endif
        assert(currentBlockOffsetPtr != nullptr);
//        assert(currentBlockOffsetPtr <= memoryBlocks.front().get());

        if(currentBlockOffsetPtr+n > currentBlockIt->get() + blockSize) {
#ifndef NDEBUG
            std::cout << "ChainedBlockAllocator::allocate(): n=" << n << std::endl;
#endif

            // current block is too small
            // either n is too big (n > _BLOCK_SIZE) -> allocate bigger block and use it
            // else next block is already allocated -> use it
            // or not -> allocate new block

            // try to use next block
            if(currentBlockIt != memoryBlocks.end() and n <= blockSize) {
                ++currentBlockIt;   // next block
                currentBlockOffsetPtr = currentBlockIt->get();
            } else {
                // need a new block -> allocate
                std::size_t size = blockSize; // default size
                if(n > blockSize) {
                    // need a bigger block (blockSize * n)
                    size = blockSize * static_cast<const std::size_t>(std::ceil(static_cast<float>(n)/blockSize));
                }
                allocateNewBlock(size);
            }
        }

        _T* p = currentBlockOffsetPtr;
        currentBlockOffsetPtr += n;
        return p;
    }

    void deallocate(pointer p, size_type n) {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::deallocate(" << p << ", " << n << ")" << std::endl;
#endif
        // TODO: push back to memoryBlocks
    }


private:
    // TODO: check bench with https://en.cppreference.com/w/cpp/types/aligned_storage
    const uint64_t initialNumberOfChunks;
    const std::size_t blockSize;
    std::forward_list<std::unique_ptr<_T[]>> memoryBlocks;  // forward chain for all blocks
    typename std::forward_list<std::unique_ptr<_T[]>>::iterator currentBlockIt;     // iterator to current block
    _T* currentBlockOffsetPtr;      // pointer to current block offset


    void allocateNewBlock(const std::size_t size=DEFAULT_BLOCK_SIZE) {
        assert(size%blockSize==0);
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::allocateNewBlock(" << size << ")" << std::endl;
#endif
        memoryBlocks.emplace_front(new _T[size]);    // allocate new block
        currentBlockOffsetPtr = memoryBlocks.front().get();
    }
};



template <typename _T>
class StaticChainedBlockAllocator {
private:
    using BlockList = std::forward_list<std::unique_ptr<_T[]>>;
    using blockListIndex_t = uint8_t;
    static constexpr blockListIndex_t BLOCK_LIST_SIZE = 64;

public:
    typedef _T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _T* pointer;
    typedef const _T* const_pointer;
    typedef _T& reference;
    typedef const _T& const_reference;


    StaticChainedBlockAllocator() : blockListArray(new BlockList[BLOCK_LIST_SIZE]) {
#ifndef NDEBUG
        std::cout << "StaticChainedBlockAllocator::StaticChainedBlockAllocator()" << std::endl;
#endif

//        init();
    }

    StaticChainedBlockAllocator(const StaticChainedBlockAllocator&) : StaticChainedBlockAllocator() {
    }

    StaticChainedBlockAllocator(StaticChainedBlockAllocator&&) = delete;

    ~StaticChainedBlockAllocator() noexcept {
#ifndef NDEBUG
        std::cout << "StaticChainedBlockAllocator::~StaticChainedBlockAllocator()" << std::endl;
#endif
        for(blockListIndex_t i=0; i<BLOCK_LIST_SIZE; ++i) {
            blockListArray[i].clear();
        }
    };


    pointer allocate(size_type n, StaticChainedBlockAllocator<_T>::const_pointer hint=nullptr) {
#ifndef NDEBUG
        std::cout << "StaticChainedBlockAllocator::allocate(n=" << n << ")" << std::endl;
#endif

        blockListIndex_t index = indexFromBlockSize(n);
        auto& blockList = blockListArray[index];

        if(blockList.empty()) {
            allocateNewBlock(index, n);
        }

        _T* p = blockList.front().release();
        assert(nullptr != p);
        blockList.pop_front();
        return p;
    }

    void deallocate(pointer p, size_type n) {
#ifndef NDEBUG
        std::cout << "StaticChainedBlockAllocator::deallocate(" << static_cast<void*>(p) << ", " << n << ")" << std::endl;
#endif
        blockListIndex_t index = indexFromBlockSize(n);
        size_t blockSize = blockSizeFromIndex(index);
        blockListArray[index].emplace_front(new (p) _T[blockSize]);
    }

private:
    void allocateNewBlock(const blockListIndex_t index, const size_t size) {
        assert(index < BLOCK_LIST_SIZE);
        const size_t blockSize = blockSizeFromIndex(index);
#ifndef NDEBUG
        std::cout << "StaticChainedBlockAllocator::allocateNewBlock(size=" << size << "): blockSize=" << size << std::endl;
#endif

        blockListArray[index].emplace_front(new _T[blockSize]);
    }

    blockListIndex_t indexFromBlockSize(size_t size) const noexcept {
        blockListIndex_t index=0;
        // size >> index
        while(size >>= 1) {
            index++;
        }

        assert(index < BLOCK_LIST_SIZE);
        return index;
    }

    size_t blockSizeFromIndex(const blockListIndex_t index) const noexcept {
#ifndef NDEBUG
        std::cout << "StaticChainedBlockAllocator::blockSizeFromIndex(index=" << (unsigned)index << "): blockSize=" << (unsigned)((size_t)1 << index) << std::endl;
#endif
        return (size_t)1 << index;
    }

    void init(const blockListIndex_t maxIndex=BLOCK_LIST_SIZE/2) {
        for(blockListIndex_t i=0; i<maxIndex; ++i) {
            auto s = blockSizeFromIndex(i);
            allocateNewBlock(i, s);
        }
    }


private:
    std::unique_ptr<BlockList[]> blockListArray;
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
