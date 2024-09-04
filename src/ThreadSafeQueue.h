#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <deque>
#include <mutex>

template<class T>
class ThreadSafeQueue {
protected:
    std::deque<T>   buffer;
    std::mutex      mutex;

public:

    void push(const T &item) {
        mutex.lock();
        buffer.push_back(item);
        mutex.unlock();
    }

    void push(T &&item) {
        mutex.lock();
        buffer.push_back(std::move(item));
        mutex.unlock();
    }

    template<class... ARGS>
    void emplace(ARGS &&... args) {
        mutex.lock();
        buffer.emplace_back(std::forward<ARGS>(args)...);
        mutex.unlock();
    }

    void pop() {
        mutex.lock();
        buffer.pop_front();
        mutex.unlock();
    }

    const T &front() const { return buffer.front(); }

    T &front() { return buffer.front(); }

    bool empty() { return buffer.empty(); }

    void clear() { buffer.clear(); }
};

#endif

