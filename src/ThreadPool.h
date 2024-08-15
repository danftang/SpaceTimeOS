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
        boost::asio::post(pool,std::forward<T>(runnable));
    }

    template<class FUNC, class RTN>
    std::future<RTN> getFuture(FUNC &&function) {
        return boost::asio::post(pool, boost::asio::use_future(std::forward<FUNC>(function)));
    }

};

extern ThreadPool executor;

#endif
