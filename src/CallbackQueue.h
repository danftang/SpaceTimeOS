#ifndef CALLBACKQUEUE_H
#define CALLBACKQUEUE_H

#include <functional>

#include "ThreadSafeQueue.h"

// A thread-safe queue of runnable functions, used to hold callbacks of
// agents that are curently blocking on another agent. 
class CallbackQueue {
private:
    ThreadSafeQueue<std::function<void()>>  callbacks;

public:
    ~CallbackQueue() { execCallbacks(); }

    template<std::convertible_to<std::function<void()>> LAMBDA>
    void pushCallback(LAMBDA &&callback) {
        callbacks.emplace(std::forward<LAMBDA>(callback));
    }

    void execCallbacks() {
        while(!callbacks.empty()) {
            callbacks.front()();
            callbacks.pop();
        }
    }
};

#endif
