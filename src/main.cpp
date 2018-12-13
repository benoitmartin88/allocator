#include <iostream>
#include <vector>

#include "memory.h"



int main() {
    std::cout << "Hello, World!" << std::endl;

    {
        std::cout << "std::allocator" << std::endl;
        auto v1 = std::vector<int, std::allocator<int>>{1,2,3};
        for(auto& v : v1) {
            std::cout << v << std::endl;
        }
    }


    {
        std::cout << "root88::memory::LinearAllocator" << std::endl;
        auto v2 = std::vector<int, root88::memory::LinearAllocator<int>>{1,2,3};
        for(auto& v : v2) {
            std::cout << v << std::endl;
        }
    }

    {
        std::cout << "root88::memory::LinearAllocator::allocate" << std::endl;
        root88::memory::LinearAllocator<int> allocator(31250);
        auto ptr = allocator.allocate(1);
        std::cout << "&ptr=" << ptr << std::endl;

        int* i = new(ptr)(int[1]);
        std::cout << "&i=" << i << std::endl;

    }


    return 0;
}
