#include <iostream>
#include <queue>
#include <functional>
#include <vector>

// #include "../src/Concepts.h"
// #include "../src/Minkowski.h"
// #include "../src/ThreadPool.h"
// #include "../src/Laboratory.h"



template<auto LAMBDA>
class MyTClass {
public:
    static auto &getLambda() { return LAMBDA; }

    template<class T>
    void execute(const T &arg) {
        LAMBDA(arg);
    }

    void myFunc() {
        LAMBDA.i = 3456;
    }
};


class MyClass {
protected:
public:
    typedef int MyType;

    static inline int y = 1234;

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
};

class MyConstClass {
public:
    static inline int i = 1234;
    const int j=0;

    void myFunc() const { std::cout << "Hello\n"; }
};


int main() {

    MyConstClass myObj;

    MyTClass<myObj> myTObj;

    myTObj.myFunc();

    return 0;
}