#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackQueue.h"
#include "Channel.h"


// TODO: Abstract over trajectory, so it's a function from time to spacetime point and a timeToIntersection with a lambda.\
// (or, perhaps point of intersection [the points should always have a frame-invariant ordering])
// ...or, since an agent must have a position at an event, the trajectory defines an offset over time (v*t)
//    so we're only abstracting over velocity:
//      - velocity * time = offset
//      - position + offset = position
//      - offset / velocity = time


// TODO:
//  - ensure position access is thread-safe
//  - encapsulate pos/vel into a trajectory so that we don't need to speicalise the 1D case.
// Base class for all agents without reference to the derived type of the agent.
template<Simulation ENV>
class Agent : public CallbackQueue {
public:
    typedef ENV::SpaceTime  SpaceTime;
    typedef ENV::Scalar     Scalar;
    typedef ENV             Environment;
    typedef ENV::Trajectory Trajectory;


    Agent(const Agent<ENV> &other) = delete; // Just don't copy objects
    Agent(Agent<ENV> &&other) = delete;

    // Construct with current active agent's trajectory
    Agent() : trajectory(activeAgent->trajectory) {
        sendCallbackTo(*activeAgent);
    }

    virtual ~Agent() {} // necessary to delete an agent without knowing the derived typw

    const SpaceTime &position() const { return trajectory.origin(); }

//    void moveForward(Scalar time) { pos += vel * time; }


//    static constexpr Scalar REACTIONTIME = 1; // local time between absorbtion of a lambda and emission of resulting particles (should this be in SpatialFunction?).

    // Attaches a ChannelReader to this object.
    void attach(ChannelExecutor<ENV> &&inChan) {
        if(trajectory.timeToIntersection(inChan.position()) < 0) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
        inChannels.push_back(std::move(inChan));
    }

    // returns a reference to an inCahnnel.
    ChannelExecutor<ENV> &getInChannel(size_t index) { return inChannels[index]; }

    // returns the number of in Channels.
    size_t nChannels() { return inChannels.size(); }

    // Closes a given inChannel.
    // invalidates inChannels.back() and inChannels.end()
    void detach(std::vector<ChannelExecutor<ENV>>::iterator channelIt) {
        if(&*channelIt != &inChannels.back()) *channelIt = std::move(inChannels.back());
        inChannels.pop_back();
    }

    // Jumps to a given point (which must be in an agent's future light-cone)
    // Once there, executes any lambdas that it now intersects, in order of
    // increasing age of the channel, and increasing age of the lambda.
    void jumpTo(const SpaceTime &newPosition) {
        if(!(position() < newPosition)) throw(std::runtime_error("An agent can only jumpTo a positions that are in its future light-cone"));
        trajectory.jumpTo(newPosition);
        for(auto it = inChannels.rbegin(); it != inChannels.rend(); ++it) {
            while(it->position() < newPosition) it->executeNext(*this);
        }
        execCallbacks();
    }


    // Execute this objects lambdas until it blocks
    void step() {
        activeAgent = this; // set this to the active agent so all lambdas know where they are.
        bool notBlocking = true;
        typename decltype(inChannels)::iterator earliestChanIt;
        do {
            earliestChanIt = moveToEarliestChannel();
            if(earliestChanIt != inChannels.end()) {
                notBlocking = earliestChanIt->executeNext(*this);
            } else {
                notBlocking = false;
            }
        } while(notBlocking);
        this->execCallbacks();
        if(earliestChanIt != inChannels.end()) {
            sendCallbackTo(*earliestChanIt);
        } else {
            // no inChannels or hit end of simulation
            if(inChannels.empty()) {
                std::cout << "Out of inCahnnels, deleting " << this << std::endl;
                delete(this); return; // no more inChannels
            } else {
                ENV::boundary.execute(*this); // hit the boundary
                return; // paranoia
            }
        }
    }

    // Kills this agent by deleting all inChannels.
    // This will signal the end of the current step
    // which will then delete this object.
    void die() { inChannels.clear(); }

    
private:

    std::vector<ChannelExecutor<ENV>>   inChannels;
    ENV::Trajectory                     trajectory;


    Agent(const Trajectory &trajectory) : trajectory(trajectory) {
        // SpaceTime displacementFromParent = trajectory.origin() - ENV::activeAgent->position();
        // if(!(ENV::activeAgent->position() < trajectory.origin()))
        //     throw(std::runtime_error("Can't construct an agent outside the future light-cone of the point of construction"));
        // sendCallbackTo(*ENV::activeAgent);
    }

    static inline Agent<ENV>          initialisingAgent = Agent<ENV>(Trajectory(SpaceTime::BOTTOM));
    static inline thread_local Agent<ENV> *activeAgent = &initialisingAgent; // each thread has an active agent on which it is currently running

    friend ENV;

    template<class DESTINATION>
    void sendCallbackTo(DESTINATION &blockingAgent) {
        blockingAgent.pushCallback([me = this]() {
                ENV::submit([me]() {
                    me->step();
                });
        });
    }


    // Access the derived type
    // inline T &derived() { return *static_cast<T *>(this); }

    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    auto moveToEarliestChannel() {
        auto chanIt = inChannels.begin();
        auto end = inChannels.end();
        auto earliestChanIt = inChannels.end();
        Scalar earliestIntersectionTime = trajectory.timeToIntersection(ENV::boundary);
        while(chanIt != end) {
            if(chanIt->isClosed()) {
                --end;
                if(chanIt != end) *chanIt = std::move(*end);
            } else {
                auto intersectTime = trajectory.timeToIntersection(chanIt->position());
                if(intersectTime <= earliestIntersectionTime) {
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                }
                ++chanIt;
            }
        }
        if(earliestIntersectionTime > 0) trajectory.advanceBy(earliestIntersectionTime);
        if(earliestChanIt == inChannels.end()) {
            inChannels.erase(end, inChannels.end());
            earliestChanIt = inChannels.end(); // new end pointer
        } else {
            inChannels.erase(end, inChannels.end());
        }
        return earliestChanIt;
    }
};


// Use this class to turn a class that isn't derived from Agent<> into an Agent.
// The underlying class can be accessed through the 'object' member
// or by deference.
template<class CLASSTOWRAP, class ENV>
class AgentWrapper : public Agent<ENV> {
protected:
    // template<std::convertible_to<typename ENV::SpaceTime> P, std::convertible_to<typename ENV::SpaceTime> V, class... ARGS>
    // AgentWrapper(P &&position, V &&velocity, ARGS &&... args) : 
    //     Agent<ENV>(std::forward<P>(position), std::forward<V>(velocity)),
    //     object(std::forward<ARGS>(args)...) {};

public:

    template<class... ARGS>
    AgentWrapper(ARGS &&... args) : object(std::forward<ARGS>(args)...) {};

    CLASSTOWRAP &operator ->() { return object; }
    const CLASSTOWRAP &operator ->() const { return object; }

    CLASSTOWRAP object;
};


// Use this class to turn a class that isn't derived from Agent<> into an Agent.
// The unserlying class is mixed-in with the Agent<> class, so members of
// each can be directly accessed.
// T should be the type we want to wrap
// ENV should be the simulation type.
template<class T, class ENV>
class AgentMixin : public Agent<ENV>, T {
public:
    template<std::convertible_to<typename ENV::SpaceTime> P, std::convertible_to<typename ENV::SpaceTime> V, class... ARGS>
    AgentMixin(ARGS &&... args) : T(std::forward<ARGS>(args)...) {};
};

#endif
