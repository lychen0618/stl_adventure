#include <iostream>
#include <algorithm>
#include <functional>
#include "sad_hashtable.h"

using namespace sad;

template <typename T>
struct identity : public std::unary_function<T, T>
{
    const T &operator()(const T &x) const { return x; }
};

template <typename T>
struct equal_to : public std::binary_function<T, T, bool>
{
    bool operator()(const T &x, const T &y) const { return x == y; }
};

typedef hashtable<int, int, std::hash<int>, identity<int>, equal_to<int>> hashtable_type;

int main()
{
    hashtable_type iht(50, std::hash<int>(), equal_to<int>());
    std::cout << iht.size() << std::endl;
    std::cout << iht.bucket_count() << std::endl;

    iht.insert_unique(59);
    iht.insert_unique(63);
    iht.insert_unique(2);
    std::cout << iht.size() << std::endl;

    hashtable_type::iterator iter = iht.begin();
    for (int i = 0; i < iht.size(); ++iter)
    {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;
    return 0;
}