#ifndef CALLBACKQUEUE_H
#define CALLBACKQUEUE_H

#include <functional>

#include "ThreadSafeQueue.h"
#include "Concepts.h"

template<Simulation ENV> class Agent;

// A thread-safe queue of runnable functions, used to hold callbacks of
// agents that are curently blocking on another agent. 
template<class ENV>
class CallbackQueue {
private:
    ThreadSafeQueue<Agent<ENV> *>  callbacks;

public:
    ~CallbackQueue() { execCallbacks(); }

    void pushCallback(Agent<ENV> *agentToCallback) {
        callbacks.emplace(agentToCallback);
    }

    // executes all the callbacks on the callback queue
    void execCallbacks() {
        while(!callbacks.empty()) {
            ENV::submit([agent = callbacks.front()]() {
                    agent->step();
            });
            callbacks.pop();
        }
    }

    // deletes all the agents on the callback queue
    void deleteCallbackAgents() {
        while(!callbacks.empty()) {
            delete(callbacks.front());
            callbacks.pop();
        }
    }
};

#endif
