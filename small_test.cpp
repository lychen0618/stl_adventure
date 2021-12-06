#include <iostream>
#include <string>
#include <list>
using namespace std;

int main(){
    if(__has_trivial_destructor(int)) cout << "string type has trivial_destructor\n";
    else cout << "string type does not have trivial_destructor\n";
    return 0;
}