#ifndef AGENT_H
#define AGENT_H

#include "Concepts.h"
#include "CallbackChannel.h"
#include "Channel.h"


// TODO:
//  - ensure position access is thread-safe
//  - encapsulate pos/vel into a trajectory so that we don't need to speicalise the 1D case.
// Base class for all agents without reference to the derived type of the agent.
template<Simulation ENV>
class Agent : public ENV::Trajectory {
public:
    typedef ENV::SpaceTime  SpaceTime;
    typedef ENV::Time       Time;
    typedef ENV             Environment;
    typedef ENV::Trajectory Trajectory;



    Agent(const Agent<ENV> &other) = delete; // Just don't copy objects
    Agent(Agent<ENV> &&other) = delete;

    // Construct with current active agent's trajectory
    Agent() : Trajectory(*activeAgent) {
        // this will be called by activeAgent so no need to lock
        activeAgent->callbackChannel.pushCallback(activeAgent->position(), this);
    }

    // needs to be virtual so we can delete an agent without knowing the derived type
    virtual ~Agent() {
        if(this == &boundaryAgent) callbackChannel.deleteCallbackAgents();
        
    } 

    // Attaches a ChannelReader to this object.
    void attach(ChannelExecutor<ENV> &&inChan) {
        if(this->timeToIntersection(inChan.position()) < 0) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
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
        Trajectory::jumpTo(newPosition);
        for(auto it = inChannels.rbegin(); it != inChannels.rend(); ++it) {
            while(it->position() < newPosition) it->executeNext(*this);
        }
    }


    // Execute this objects lambdas until it blocks
    void step() {
        activeAgent = this; // set this to the active agent so all lambdas know where they are.
        while(executeNextLambda()) { };
        callbackChannel.updatePosition(this->position());
        if(inChannels.empty()) {
            std::cout << "Out of inCahnnels, deleting " << this << std::endl;
            delete(this); return; // no more inChannels
        }
    }

    // Kills this agent by deleting all inChannels.
    // This will signal the end of the current step
    // which will then delete this object.
    void die() { inChannels.clear(); }


    // Starts simulation of all agents in environment ENV by
    // submitting callbacks of all agents on the boundaryAgent.
    static void start() {
        boundaryAgent.advanceBy(boundaryAgent.timeToIntersection(ENV::boundary));
        boundaryAgent.callbackChannel.updatePosition(boundaryAgent.position());
    }


    CallbackChannel<ENV>                callbackChannel;
    static inline Agent<ENV>            boundaryAgent = Agent<ENV>(Trajectory(SpaceTime::BOTTOM));
private:

    std::vector<ChannelExecutor<ENV>>   inChannels;

    static inline thread_local Agent<ENV> *activeAgent = &boundaryAgent; // each thread has an active agent on which it is currently running

    friend ENV;

    Agent(const Trajectory &trajectory) : Trajectory(trajectory) {
        // SpaceTime displacementFromParent = trajectory.origin() - ENV::activeAgent->position();
        // if(!(ENV::activeAgent->position() < trajectory.origin()))
        //     throw(std::runtime_error("Can't construct an agent outside the future light-cone of the point of construction"));
        // sendCallbackTo(*ENV::activeAgent);
    }


    // Access the derived type
    // inline T &derived() { return *static_cast<T *>(this); }

    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    bool executeNextLambda() {
        bool didExecution;
        auto chanIt = inChannels.begin();
        auto end = inChannels.end();
        auto earliestChanIt = inChannels.end();
        SpaceTime earliestChanPos;
        Time earliestIntersectionTime = this->timeToIntersection(ENV::boundary);
        while(chanIt != end) {
            if(chanIt->isClosed()) { 
                --end; // mark closed channels for deletion without invalidating the end() iterator
                if(chanIt != end) *chanIt = std::move(*end);
            } else {
                SpaceTime chanPosition = chanIt->position();
                auto intersectTime = this->timeToIntersection(chanPosition);
                if(intersectTime <= earliestIntersectionTime) {
                    earliestChanPos = chanPosition;
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                }
                ++chanIt;
            }
        }
        if(earliestIntersectionTime > 0) this->advanceBy(earliestIntersectionTime);
        if(earliestChanIt == inChannels.end()) {
            // earliest channel is boundary
            didExecution = false;
            boundaryAgent.callbackChannel.pushCallback(boundaryAgent.position(), this);
        } else {
            didExecution = earliestChanIt->executeNext(*this);
            if(!didExecution) earliestChanIt->pushCallback(earliestChanPos, this);
        }
        inChannels.erase(end, inChannels.end()); // delete closed channels
        return didExecution;
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
