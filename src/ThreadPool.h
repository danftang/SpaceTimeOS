#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <thread>
#include <boost/asio.hpp>
#include <queue>
#include <iostream>

template<uint NTHREADS>
class ThreadPool {
protected:
    boost::asio::thread_pool pool;

public:
    ThreadPool() : pool(NTHREADS) {}

    ~ThreadPool() {
        pool.join();
    }

    template<class T>
    void submit(T &&runnable) {
        boost::asio::post(pool,std::forward<T>(runnable));
    }

    template<class FUNC, class RTN>
    std::future<RTN> getFuture(FUNC &&function) {
        return boost::asio::post(pool, boost::asio::use_future(std::forward<FUNC>(function)));
    }
};

template<>
class ThreadPool<0> {
protected:
    std::queue<std::function<void()>> tasks;
public:

    template<class T>
    void submit(T &&runnable) {
        tasks.push(std::forward<T>(runnable));
    }

    void start() {
        std::cout << "Starting exec with " << tasks.size() << " tasks" << std::endl;
        while(!tasks.empty()) {
            tasks.front()();
            tasks.pop();
            std::cout << tasks.size() << " tasks" << std::endl;
        }
        std::cout << "Done" << std::endl;
    }
};

extern ThreadPool<0> executor;

#endif