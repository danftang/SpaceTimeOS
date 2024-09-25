#ifndef CALLBACKQUEUE_H
#define CALLBACKQUEUE_H

#include <functional>

#include "ThreadSafeQueue.h"

// A thread-safe queue of runnable functions, used to hold callbacks of
// agents that are curently blocking on another agent. 
class CallbackQueue {
protected:
    ThreadSafeQueue<std::function<void()>>  callOnMove;

public:
    ~CallbackQueue() { execAll(); }

    template<std::convertible_to<std::function<void()>> LAMBDA>
    void push(LAMBDA &&callback) {
        callOnMove.emplace(std::forward<LAMBDA>(callback));
    }

    void execAll() {
        while(!callOnMove.empty()) {
            callOnMove.front()();
            callOnMove.pop();
        }
    }
};

#endif
