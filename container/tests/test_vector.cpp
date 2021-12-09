#include <iostream>
#include "sad_vector.h"

using namespace sad;

int main(){
    vector<int> vec(5, 1);
    for(int i = 0; i < 10; ++i) vec.push_back(2);
    for(auto &num : vec){
        std::cout << num << " ";
    }
    std::cout << std::endl;

    vec.insert(vec.begin() + 3, 20, 3);
    for(auto &num : vec){
        std::cout << num << " ";
    }
    std::cout << std::endl;
    return 0;
}