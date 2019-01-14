# C++ Allocator

STL compliant allocator.

These allocators can be used with any STL containers.

# Example: std::vector
``` c++
using Allocator = StaticChainedBlockAllocator;
using T = char;

auto v = std::vector<_T, Allocator<T>>(SIZE);
for(size_t i=0; i<10; ++i) {
    std::cout << "emplace_back i=" << i << std::endl;
    v.emplace_back(val);
}
```


# Example: placement new
``` c++
static constexpr uint8_t SIZE = 8;
root88::memory::StaticBlockAllocator<int> allocator;  // 8 * sizeof(int)
auto ptr = allocator.allocate(SIZE);

for(uint8_t i=0; i<SIZE; ++i) {
    int* intPtr = new(ptr+i)(int);
    *intPtr = i;    // set pointer value
}

int* intPtr = new(ptr)(int[SIZE]);
for(uint8_t i=0; i<SIZE; ++i) {
    *(intPtr+i) = i;    // set pointer value
}

allocator.deallocate(ptr, SIZE);    // explicit call to deallocate due to new placement
```




