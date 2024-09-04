#include <iostream>
#include <queue>
#include <functional>
#include <vector>

template<class T>
class MyTClass {
protected:
    void myFunc() {}
public:
    friend T;  
};


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

    void myFunc() {
        MyTClass<MyClass> x;
        x.myFunc();
    }

};


int main() {
    MyClass x;

    x.myFunc();

    std::cout << "Hello" << std::endl;
    return 0;
}