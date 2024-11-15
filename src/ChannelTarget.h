#ifndef CHANNELTARGET_H
#define CHANNELTARGET_H

// Minimal functionality needed for an agent that is a target of a channel
// Target also needs to know position and velocity to do any execution...
// This could be better as a concept (anything that has attach() can be a target)
// then an agent can be a subclass of ChannelSource
template<class ENV>
class ChannelTarget {
public:

    // Attaches a ChannelExecutor to this object.
    void attach(ChannelExecutor<ENV> &&inChan) {
//        if(this->timeToIntersection(inChan.asField()) < 0) throw(std::runtime_error("Attempt to attach channel to an agent's past"));
        inChannels.push_back(std::move(inChan));
    }

    // Closes a given inChannel.
    // invalidates inChannels.back() and inChannels.end()
    void detach(std::vector<ChannelExecutor<ENV>>::iterator channelIt) {
        if(&*channelIt != &inChannels.back()) *channelIt = std::move(inChannels.back());
        inChannels.pop_back();
    }

    // returns time to intersection and iterator to the earliest channel
    // If iterator is end() then boundary is the earliest. If the channel
    // is empty then we're blocking.
    // ...or return new position and iterator
    std::pair<Time, std::vector<ChannelExecutor<ENV>>::iterator> getEarliestChannel(SpaceTime pos, Velocity vel) {
    }

protected:
    std::vector<ChannelExecutor<ENV>>   inChannels;


};


#endif
