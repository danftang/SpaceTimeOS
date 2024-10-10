#ifndef CALLBACKQUEUE_H
#define CALLBACKQUEUE_H

#include <functional>

#include "ThreadSafeQueue.h"
#include "Concepts.h"

template<Simulation ENV> class Agent;

// A thread-safe queue of runnable functions, used to hold callbacks of
// agents that are curently blocking on another agent.
//
// The CallbackChannel is effectively a many to one two-way comms channel
// that passes positions from the unique agent to all agents that it
// has channels to, and accepts callbacks from them. 
//
// TODO: solve these problems:
// The read-request problem:
//   - agent A reads B's position as p1 and blocks there
//   - B moves to p2 and clears his callback queue
//   - A sends a request to B to callback on moving from p1
//   - A doesn't get a callback until B moves from p2.
// The move-exec problem:
//   - agent B moves from p1 to p2
//   - Agent A reads B's new position and blocks, sending a callback for p2
//   - agent B execsCallbacks for p1...and callsback B's callback for p2
// The deleted-source problem:
//   - source is deleted.
//   - If the CallbackChannel is deleted before any of the source's outChannels
//     then there's a chance that the client will try to write to a deleted CallbackChannel.
//     (does C++ guarantee that a derived class's members are deleted before those of the base class?
//      what about the mixin class?)
// 
// If the CallbackChannel has a position which is the position at which all
// agents will block on (i.e. an upper bound for the targets, and a lower bound for
// the source).
// When the source blocks, it...
//   - locks the queue
//   - updates the queue's position and swaps out the queue for a new queue
//   - unlocks the queue
//   - submits the callbacks from the old queue
// When a client requests a callback it:
//   - locks the queue
//   - checks the lab-time of its blocking position, t_b, against that of the queue, t_q
//   - if (t_b < t_q) submit the callback immediately
//     else push the callback onto the queue
//   - unlock the queue 
// then all is well.
//
// On the source side, we just need an updatePosition() method.
// On the client side we we need a getPosition() [wrapped by the channel] and submitCallback(labtime, agent)
//   
template<class ENV>
class CallbackChannel {
public:
    typedef typename ENV::SpaceTime SpaceTime;
    typedef typename ENV::SpaceTime::Time Time;


    CallbackChannel() : pBuffer(new std::deque<Agent<ENV> *>()) { }

    ~CallbackChannel() {
        // by this time all channels should be closed so no need to lock
        execCallbacks(pBuffer);
        delete(pBuffer);
    }

    // To be called from the source
    void updatePosition(const SpaceTime &newPosition) {
        std::deque<Agent<ENV> *> *newBuffer = new std::deque<Agent<ENV> *>();
        std::deque<Agent<ENV> *> *oldBuffer;
        mutex.lock();
            pos = newPosition;
            oldBuffer = pBuffer;
            pBuffer = newBuffer;
        mutex.unlock();
        execCallbacks(oldBuffer);
        delete(oldBuffer);
    }

    // deletes all the agents on the callback queue
    void deleteCallbackAgents() {
        while(!pBuffer->empty()) {
            std::cout << "Deleting agent " << pBuffer->front() << " from boundaryAgent callbacks" << std::endl;
            delete(pBuffer->front());
            pBuffer->pop_front();
        }
    }

    // To be called by the targets
    
    SpaceTime position() {
        SpaceTime position;
        mutex.lock();
            position=pos;
        mutex.unlock();
        return std::move(position);
    }

    void pushCallback(const SpaceTime &callbackOnMoveFromPosition, Agent<ENV> *agentToCallback) {
        bool executeNow = false;
        mutex.lock();
            if(callbackOnMoveFromPosition.labTime() < pos.labTime()) executeNow = true; else pBuffer->push_back(agentToCallback);
        mutex.unlock();
        if(executeNow) execCallback(agentToCallback);
    }
    
protected:
    std::deque<Agent<ENV> *> *  pBuffer;
    std::mutex                  mutex;
    SpaceTime                   pos;

    void execCallbacks(std::deque<Agent<ENV> *> *buffer) {    
        while(!buffer->empty()) {
            execCallback(buffer->front());
            buffer->pop_front();
        }
    }

    inline void execCallback(Agent<ENV> *agent) {
        ENV::submit([agent]() {
            agent->step();
        });
    }

};

#endif
