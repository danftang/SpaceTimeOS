#ifndef AGENT_H
#define AGENT_H

#include <limits>

#include "Concepts.h"
#include "SourceAgent.h"
#include "Channel.h"
#include "LinearTrajectory.h"
#include "deselbystd/random.h"


// An Agent is not much more than a vector field with registered writers
// which is expressible as the sum of (exponential) functions from a parameterised 
// family of functions, where the parameter is position in the field.
// Each writer has a presence in the field which is in the family,
// and whose position may change.
//
// At its most basic, a field gives a value, gradient and 2nd order gradient
// matrix for any point.
// Ultimately we want to turn a trajectory to a time/position of intersection and
// a lambda which is intersected.
//
// The field would be a good place to define the blocking field/lambda fields
// If we assume the field type itself defines the velocity ordering
// and that the lambda field has a space-like tangent linear model at
// all points on its zero surface, then the lambda field is its own
// blocking field.
// 
// ...and a good distinction between the field and the points of a field.
//
// A derived agent is both a field reader and writer, and may write
// to many fields.
//
// If velocity is constant (or the field is 1-Dimensional) then
// the trajectory is fixed and imposes a complete order on the field and
// so we can pull events out of the channels and put them into a
// queue.
//
 
//
// Base class for all agents
// 
template<Environment ENV>
class Agent : public SourceAgent<ENV> {
public:
    typedef ENV::SpaceTime  SpaceTime;
    typedef ENV::SpaceTime::Time       Time;
    typedef ENV             Environment;



    Agent(const Agent<ENV> &other) = delete; // Just don't copy objects
    Agent(Agent<ENV> &&other) = delete;

    // Construct with current active agent's trajectory
    Agent() : SourceAgent<ENV>(*Simulation<ENV>::currentThreadAgent) {
        // this will be called on currentThreadAgent's thread so no need to lock
        Simulation<ENV>::currentThreadAgent->pCallbackBuffer->push(this);
    }

    virtual ~Agent() {} // virtual so that we can delete agents on a callback queue.

    // needs to be virtual so we can delete an agent without knowing the derived type

    // Attaches a ChannelReader to this object.
    void attach(ChannelExecutor<ENV> &&inChan) {
        if(this->timeToIntersection(inChan.getCallbackField()->asBlockingField()) < 0) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
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

    // Jumps to a given point in spacetime (which must be in an agent's future light-cone)
    // without passing through any intermediate points.
    // Once there, executes any lambdas that it now intersects in order of earliest
    // at the current velocity (in its past) without moving.
    void jumpTo(const SpaceTime &newPosition) {
        assert(this->position() < newPosition);
        this->updatePosition(newPosition);
    }


    // Execute this objects lambdas until it blocks
    void step() {
        Simulation<ENV>::currentThreadAgent = this; // set this to the active agent so all lambdas know where they are.
        std::shared_ptr<CallbackField<ENV>> blockingQueue;
        do {
            blockingQueue = executeNextLambda();
        } while(!blockingQueue);
        if(inChannels.empty()) {
            std::cout << "Out of inCahnnels, deleting " << this << std::endl;
            delete(this); return; // no more inChannels
        }
        blockingQueue->push(this); // push ourselves onto the queue of the agent we're blocking on and return immediately
    }


    // Kills this agent by deleting all inChannels.
    // This will signal the end of the current step
    // which will then delete this object.
    void die() { inChannels.clear(); }






    // This is the agent on which notionally runs the main thread that starts/ends the computation.
//    static inline Agent<ENV>            mainThreadAgent = Agent<ENV>(Trajectory(-sqrt(std::numeric_limits<typename SpaceTime::Time>::max())));
private:

    std::vector<ChannelExecutor<ENV>>   inChannels;

    // TODO: this need only be a callback field, could initially be the boundary (though this would be of a different type, damn)

    // friend ENV;

    // Agent(const Trajectory &trajectory) : Trajectory(trajectory) {
    //     // SpaceTime displacementFromParent = trajectory.origin() - ENV::currentThreadAgent->position();
    //     // if(!(ENV::currentThreadAgent->position() < trajectory.origin()))
    //     //     throw(std::runtime_error("Can't construct an agent outside the future light-cone of the point of construction"));
    //     // sendCallbackTo(*ENV::currentThreadAgent);
    // }


    // finds the earliest channel and moves this to its intersection point,
    // detaching any closed channels as it goes.
    // If inChannels is empty, moves this to SpaceTime::TOP
    std::shared_ptr<CallbackField<ENV>> executeNextLambda() {
        auto chanIt = inChannels.rbegin();
        auto earliestChanIt = inChannels.rend();
        int multiplicity = 1; // number of earliest channels found
        std::shared_ptr<CallbackField<ENV>> earliestBlockingQueue = Simulation<ENV>::mainThread.getCallbackField(); // if we block on the boundary, add ouselves back to the mainThreadAgent
        Time earliestIntersectionTime = this->timeToIntersection(Simulation<ENV>::boundary);
        while(chanIt != inChannels.rend()) {
            if(chanIt->isClosed()) {
                if(chanIt != inChannels.rbegin()) *chanIt = std::move(inChannels.back()); // remove closed channels
                ++chanIt;
                inChannels.pop_back();
            } else {
                Time intersectTime;
                std::shared_ptr<CallbackField<ENV>> pBlockingField;
                if(chanIt->empty()) {
                    pBlockingField = chanIt->getCallbackField();
                    intersectTime = this->timeToIntersection(pBlockingField->asBlockingField());
                } else {
                    intersectTime = this->timeToIntersection(chanIt->asLambdaField());
                }
                if(intersectTime < earliestIntersectionTime) {
                    earliestIntersectionTime = intersectTime;
                    earliestChanIt = chanIt;
                    earliestBlockingQueue = std::move(pBlockingField);
                    multiplicity = 1;
                } else if(intersectTime == earliestIntersectionTime && !pBlockingField) { // execute before blocking
                    if(!earliestBlockingQueue) ++multiplicity;
                    if(deselby::Random::nextDouble() < 1.0/multiplicity) { // choose between equal options with uniform random distribution
                        earliestIntersectionTime = intersectTime;
                        earliestChanIt = chanIt;
                        earliestBlockingQueue.reset();
                    }
                }
                ++chanIt;
            }
        }
        if(earliestIntersectionTime > 0) {
            this->advanceBy(earliestIntersectionTime);
        }
        if(earliestChanIt != inChannels.rend() && !earliestBlockingQueue) {
            // found a lambda so execute it
            earliestChanIt->executeNext(*this);
        }
        // Three outcomes: 
        //   - successfully executed: returns nullptr
        //   - blocked on channel: returns ptr to channel callback queue
        //   - blocked on boundary: returns ptr to boundary callback queue
        return earliestBlockingQueue; // return shared pointer to callbackQueue or null
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
