//
// Created by MARTIN Benoît on 03/01/2019.
//

#ifndef MEMORY_CHAINED_BLOCK_ALLOCATOR_H
#define MEMORY_CHAINED_BLOCK_ALLOCATOR_H

#include <memory>
#include <stdlib.h>
#include <cassert>
#include <forward_list>
#include <iostream>     // TODO remove
#include <cmath>


namespace root88 {
namespace memory {

/**
 * ChainedBlockAllocator
 * STL compliant allocator
 *
 * @tparam _T type
 *
 * Chained blocks of
 * | 1 | 2 | 4 | 8 | 16 | 32 | 64 | ... |   (1<<BLOCK_LIST_SIZE)
 *   B   B       B              B
 *   B           B
 *   B
 */
template <typename _T>
class ChainedBlockAllocator {
private:
    using BlockList = std::forward_list<std::unique_ptr<_T[]>>;
    using blockListIndex_t = uint8_t;
    static constexpr blockListIndex_t BLOCK_LIST_SIZE = 64;

public:
    class Test;
    typedef _T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _T* pointer;
    typedef const _T* const_pointer;
    typedef _T& reference;
    typedef const _T& const_reference;


    ChainedBlockAllocator() : blockListArray(new BlockList[BLOCK_LIST_SIZE]) {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::ChainedBlockAllocator()" << std::endl;
#endif
//        init();
    }

    ChainedBlockAllocator(const ChainedBlockAllocator& other) : ChainedBlockAllocator() {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::ChainedBlockAllocator(const ChainedBlockAllocator&)" << std::endl;
#endif
        for(blockListIndex_t i=0; i<BLOCK_LIST_SIZE; ++i) {
            for(auto& val : other.blockListArray[i]) {
                blockListArray[i].emplace_front(val.get());
            }
        }
    }

    ChainedBlockAllocator(ChainedBlockAllocator&&) = delete;

    ~ChainedBlockAllocator() noexcept {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::~ChainedBlockAllocator()" << std::endl;
#endif
//        for(blockListIndex_t i=0; i<BLOCK_LIST_SIZE; ++i) {
//            blockListArray[i].clear();
//        }
    };


    pointer allocate(size_type n, ChainedBlockAllocator<_T>::const_pointer hint=nullptr) {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::allocate(n=" << n << ")" << std::endl;
#endif

        blockListIndex_t index = indexFromBlockSize(n);
        auto& blockList = blockListArray[index];

        if(blockList.empty()) {
            allocateNewBlock(index);
        }

        assert(not blockList.empty());
        _T* p = blockList.front().release();
        assert(nullptr != p);
        blockList.pop_front();
        return p;
    }

    void deallocate(pointer p, size_type n) {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::deallocate(" << static_cast<void*>(p) << ", " << n << ")" << std::endl;
#endif
        blockListIndex_t index = indexFromBlockSize(n);
        size_t blockSize = blockSizeFromIndex(index);
        blockListArray[index].emplace_front(new (p) _T[blockSize]);
    }

    template <typename _U>
    struct rebind {
        typedef ChainedBlockAllocator<_U> other;
    };

    void construct(pointer p, const_reference clone) {
        new (p) _T(clone);
    }

    void destroy(pointer p) {
        p->~_T();
    }

    pointer address(reference x) const {
        return &x;
    }

    const_pointer address(const_reference x) const {
        return &x;
    }

    bool operator==(const ChainedBlockAllocator &rhs) {
        return blockListArray.get() == rhs.blockListArray.get();
    }

    bool operator!=(const ChainedBlockAllocator &rhs) {
        return !operator==(rhs);
    }


private:
    void allocateNewBlock(const blockListIndex_t index) {
        assert(index < BLOCK_LIST_SIZE);
        const size_t blockSize = blockSizeFromIndex(index);
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::allocateNewBlock(index=" << unsigned(index) << "): blockSize=" << blockSize << std::endl;
#endif

        blockListArray[index].emplace_front(new _T[blockSize]);
    }

    /**
     * Return an index to the inner blockListArray. Value will be between 0 and BLOCK_LIST_SIZE-1
     * @param size block size
     * @return index
     */
    inline blockListIndex_t indexFromBlockSize(size_t size) const noexcept {
        blockListIndex_t index=0;
        // size >> index
//        while(size >>= 1) {
//            ++index;
//        }

        for(;size >>= 1; ++index);

        assert(index < BLOCK_LIST_SIZE);
        return index;
    }

    inline size_t blockSizeFromIndex(const blockListIndex_t index) const noexcept {
#ifndef NDEBUG
        std::cout << "ChainedBlockAllocator::blockSizeFromIndex(index=" << (unsigned)index << "): blockSize=" << (size_t(1) << index) << std::endl;
#endif
        return size_t(1) << index;
    }

    void init(const blockListIndex_t maxIndex=BLOCK_LIST_SIZE/2) {
        for(blockListIndex_t i=0; i<maxIndex; ++i) {
            allocateNewBlock(i);
        }
    }


private:
    std::unique_ptr<BlockList[]> blockListArray;
};

}   // namespace allocator
}   // namespace root88

#endif //MEMORY_CHAINED_BLOCK_ALLOCATOR_H
