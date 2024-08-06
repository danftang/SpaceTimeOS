#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <thread>
#include <boost/asio.hpp>

class ThreadPool {
    boost::asio::thread_pool pool;

public:
    ThreadPool() : pool(10) {}
    ~ThreadPool() {
        pool.join();
    }

    template<class T>
    void submit(T &&runnable) {
        boost::asio::post(pool,std::move(runnable));
    }
};

extern ThreadPool executor;

#endif
