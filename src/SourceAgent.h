#ifndef CALLBACKQUEUE_H
#define CALLBACKQUEUE_H

#include <functional>
#include <atomic>

#include "Concepts.h"
#include "ThreadSafeQueue.h"
#include "TranslatedField.h"
#include "Velocity.h"
#include "predeclarations.h"
#include "numerics.h"

template<Environment ENV>
class CallbackQueue  {
public:

    ~CallbackQueue() {
        if(!isTriggered) trigger();
    }

    void push(Agent<ENV> *agent) {
        if(isTriggered) {
            execCallback(agent);
        } else {
            mutex.lock();
            buffer.push_back(agent);
            mutex.unlock();
        }
    }


protected:
    std::deque<Agent<ENV> *>   buffer;
    std::mutex      mutex;
    bool isTriggered = false;

    friend class SourceAgent<ENV>;
    friend class CallbackChannel<ENV>;

    void trigger() {
        mutex.lock();
        isTriggered = true;
        mutex.unlock(); // once triggered, the buffer is isolated from the target.
        while(!buffer.empty()) {
            execCallback(buffer.front());
            buffer.pop_front();
        }
    }

    // deletes all the agents on the callback queue
    void deleteCallbackAgents() {
        mutex.lock();
        isTriggered = true;
        mutex.unlock();
        while(!buffer.empty()) {
            std::cout << "Deleting agent " << buffer.front() << " from boundaryAgent callbacks" << std::endl;
            delete(buffer.front());
            buffer.pop_front();
        }
    }

    inline static void execCallback(Agent<ENV> *agent) {
        Simulation<ENV>::executor.submit([agent]() {
            agent->step();
        });
    }
};


template<Environment ENV>
class CallbackField : public CallbackQueue<ENV> {
protected:
    typedef Simulation<ENV>::TranslatedLambdaField TranslatedLambdaField;

    const TranslatedLambdaField lambdaField;

public:
    CallbackField() = default;
    CallbackField(TranslatedLambdaField field) : lambdaField(std::move(field)) {}
    CallbackField(ENV::SpaceTime origin) : lambdaField(Simulation<ENV>::lambdaField, std::move(origin)) { }


    // timeToBlocking(pos, vel)

    // Since the fields are const, we don't care about locking or which thread we're on.
    const auto &asBlockingField() const { return lambdaField; }   // used for blocking other agents
    const auto &asLambdaField() const { return lambdaField; }     // used for sending lambdas
    const auto &asPosition() const { return lambdaField.origin; } // used for calculating trajectories and spawning new agents
};


// A thread-safe queue of runnable functions, used to hold callbacks of
// agents that are curently blocking on another agent.
//
// The CallbackChannel is effectively a many to one two-way comms channel
// that passes positions from the unique agent to all agents that it
// has channels to, and accepts callbacks from them. 
template<Environment ENV>
class CallbackChannel {
public:

    CallbackChannel(typename ENV::SpaceTime position) {
        pCallbackBuffer = std::make_shared<CallbackField<ENV>>(std::move(position));
    }

    CallbackChannel(const CallbackChannel<ENV> &other) : CallbackChannel(other.pCallbackBuffer->asPosition()) { }

    ~CallbackChannel() {
        if(this == &Simulation<ENV>::mainThread) pCallbackBuffer->deleteCallbackAgents();
    }


    // to be called from the channel, on the source thread when sending a lambda, so no need for atomic ref
    const auto &asLambdaField() const {
        assert(Simulation<ENV>::currentThreadAgent->hasAuthorityOver(this)); // check this thread has authority over this agent
        return pCallbackBuffer->asLambdaField();
    }

    // A Channel should call this to determine the blocking field.
    // This is called from the target's thread so it 
    // needs to be threadsafe
    inline std::shared_ptr<CallbackField<ENV>> getCallbackField() {
        mutex.lock();
        std::shared_ptr<CallbackField<ENV>> copyOfPtr(pCallbackBuffer);
        mutex.unlock();
        return copyOfPtr;
    }

    // An agent has authority over itself and all the agents in its callback buffer.
    bool hasAuthorityOver(const CallbackChannel<ENV> *agent) const {
        if(agent == this) return true;
        for(Agent<ENV> *agentInQueue : pCallbackBuffer->buffer) {
            if(agent == agentInQueue) return true;
        }
        return false;
    }

protected:
    std::mutex mutex;
    std::shared_ptr<CallbackField<ENV>>   pCallbackBuffer;      // the current position's callback queue and blocking field
    // TODO: lazy creation of the buffer (perhaps nobody will ever block on us)
    //       We could do this by keeping a local copy of the position here, so that the callback buffer can be
    //       null until someone calls getCallbackField() [depends how long it takes to create a CallbackQueue and delete it
    //       and the probability of not being blocked on]
};


// A SourceAgent is a CallbackChannel with the agent-facing interface (as opposed to channel-facing interface)
template<Environment ENV>
class SourceAgent : public CallbackChannel<ENV> {
public:
    typedef typename ENV::SpaceTime         SpaceTime;
    typedef typename ENV::SpaceTime::Time   Time;

    SourceAgent(typename ENV::SpaceTime position) : CallbackChannel<ENV>(std::move(position)) { }
    SourceAgent(const SourceAgent<ENV> &other) :  CallbackChannel<ENV>(other), vel(other.vel) { }

    using CallbackChannel<ENV>::pCallbackBuffer;

    // To be called by the source agent
    void updatePosition(const SpaceTime &newPosition) {
        pCallbackBuffer->trigger();
        std::shared_ptr<CallbackField<ENV>> newBuffer = std::make_shared<CallbackField<ENV>>(newPosition);
        this->mutex.lock();
        pCallbackBuffer = std::move(newBuffer);
        this->mutex.unlock();
    }

    void advanceBy(Time time) { updatePosition(position() + vel * time); }
    
    template<FirstOrderField F>
    Time timeToIntersection(const F &field) const {
        return -field(position())/field.d_dt(vel); // should be valid integer division when time is integer
    }


    template<SecondOrderField F>
    Time timeToIntersection(const F &field) const {
        auto a = field.d2_dt2(vel);
        auto mb = -field.d_dt(position(),vel)/2;
        auto sq = mb*mb - a*field(position());
        if constexpr(std::floating_point<Time>) sq += delta(sq); // ensure we don't round into negative
        if(sq < 0) return std::numeric_limits<Time>::max(); // never intersects

        return (mb + sqrt(sq))/a;
    }

    Velocity<SpaceTime>                 vel;

    const SpaceTime &position() const {
        return this->pCallbackBuffer->asPosition();
    }
protected:

};

#endif
