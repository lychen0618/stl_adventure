#include <iostream>
#include "alloc.h"

void handler(){
    std::cout << "no more space!\n";
    static int time = 0;
    if(time > 10) exit(1);
}

int main(){
    sad::malloc_allocator<int> allocator;
    allocator.set_oom_handler(handler);
    int* arr = static_cast<int*>(allocator.allocate(20000000000));
    allocator.construct(arr, 1);
    allocator.construct(arr + 1, 2);
    std::cout << arr[0] << " " << arr[1] << std::endl;
    allocator.destroy(arr);
    allocator.destroy(arr + 1);
    allocator.deallocate(arr, 2);
    return 0;
}