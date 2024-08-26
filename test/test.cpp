#include <iostream>
#include <queue>
#include <functional>

class MyClass {
protected:
public:
    MyClass() {
        std::cout << "Creating" << std::endl;
    }

    MyClass(const MyClass &move) {
        std::cout << "Copying" << std::endl;
    }

    MyClass(MyClass &&move) {
        std::cout << "Moving" << std::endl;
    }

    ~MyClass() {
        std::cout << "Destructing" << std::endl;
    }

};

template<class A, class B>
class Channel {
    public:

    class In {
        
    };


    template<class C>
    void myFunc(const Channel<C,B> &other) {}


};


int main() {

    Channel<int,int>    myIntInt;
    Channel<double,int> myDoubleInt;
    Channel<double,int>::In myDoubleIntIn;

    myIntInt.myFunc(myDoubleInt);

    std::cout << "Hello" << std::endl;
    return 0;
}