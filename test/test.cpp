#include <iostream>
#include <queue>
#include <functional>
#include <vector>

class MyClass {
protected:
public:
    MyClass() {
        std::cout << "Creating" << std::endl;
    }

    // MyClass(const MyClass &) {
    //     std::cout << "Copying" << std::endl;
    // }

    MyClass(MyClass &&) {
        std::cout << "Moving" << std::endl;
    }

    ~MyClass() {
        std::cout << "Destructing" << std::endl;
    }

};


int main() {

    std::vector<MyClass> myV;

    myV.reserve(8);

    myV.push_back(MyClass());
    myV.push_back(MyClass());
    myV.push_back(MyClass());

    std::cout << "Hello" << std::endl;
    return 0;
}