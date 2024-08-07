#include <iostream>
#include <thread>
#include "ThreadPool.h"
#include "ReferenceFrame.h"

template<ReferenceFrame T>
void MyFunc(T obj) {
}

int main() {

    for (int i = 0; i < 10; ++i) {
        executor.submit([i]() {
            std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << std::endl;
//            std::cout << "Task " << i << std::endl;
        });
    }

    // std::future<int> r1 = boost::asio::post(pool, asio::use_future([]() { return 2; }));
    // std::cout << "Result = " << r1.get() << '\n';

    // // Optional: Wait for all tasks to complete
    // pool.join();

    std::cout << "Goodbye world" << std::endl;
    return 0;
}