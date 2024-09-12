#include <iostream>
#include <queue>
#include <functional>
#include <vector>

// #include "../src/Concepts.h"
// #include "../src/Minkowski.h"
// #include "../src/ThreadPool.h"
// #include "../src/Laboratory.h"


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

    template<class T>
    static void step(T t) {}
};

template<class T, class S> concept TestConcept = requires(S self) {
    { T::step(self) };
};

// typedef Minkowski<2>                 MySpaceTime;
// typedef Laboratory<MySpaceTime, ThreadPool<2>>      MySimulation;

class MyDerived;

class MyBase {
    private:
    int x;

public:
    friend class MyDerived;
};

class MyDerived : public MyBase {
public:
    void myFunc() {
        x += 1;
    }
};

class MyDerivedDerived : public MyDerived {
    void f() {
        x += 2;
    }
};



int main() {
    MyClass x;

    x.myFunc();

    std::cout << "Hello" << std::endl;
    return 0;
}