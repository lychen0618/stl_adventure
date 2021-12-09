#include <iostream>

void get()
{
    int e;
    int f = 4;
    static int g;
    static int h = 5;
    std::cout << "in get(), the address a is " << &e << std::endl;
    std::cout << "in get(), the address b is " << &f << std::endl;
    std::cout << "in get(), the address c is " << &g << std::endl;
    std::cout << "in get(), the address d is " << &h << std::endl;
    while (1); // 让程序死在这用来查看内存映射表
}

int main()
{

    // int a = 1;
    // int b;
    // static int c = 2;
    // static int d;
    // int *p = new int(3);
    // std::cout << "in main, the address a is " << &a << std::endl;
    // std::cout << "in main, the address b is " << &b << std::endl;
    // std::cout << "in main, the address c is " << &c << std::endl;
    // std::cout << "in main, the address d is " << &d << std::endl;
    // std::cout << "in main, the address p is " << p << std::endl;
    // get(); // 程序在这进入死循环

    // delete p;
    // p = NULL;
    int i = 0 + 10;
    printf("i = %d\n", i);
    while(true) ;
    return 0;
}
