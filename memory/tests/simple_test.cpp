#include <iostream>
#include "alloc.h"

void handler(){
    std::cout << "no more space!\n";
    static int time = 0;
    if(time > 10) exit(1);
}

typedef sad::malloc_allocator<int> allocator;
typedef sad::pool_allocator<int> double_allocator;

int main(){
    allocator::set_oom_handler(handler);
    int* arr = static_cast<int*>(double_allocator::allocate(2));
    sad::construct(arr, 4);
    sad::construct(arr + 1, 3);
    std::cout << arr[0] << " " << arr[1] << std::endl;
    sad::destroy(arr);
    sad::destroy(arr + 1);
    double_allocator::deallocate(arr, 2);
    return 0;
}