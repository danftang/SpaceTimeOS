#ifndef THREADSAFEPOSITION_H
#define THREADSAFEPOSITION_H

#include <atomic>

// Used to store a spacetime position that can be written to by the 
template<class SPACETIME>
class ThreadSafePosition {
private:
    SPACETIME pos;

protected:

    // agent writes should be atomic and shouldn't commute with writes to any channel 
    inline void setPosition(SPACETIME newPosition) { 
        std::atomic_ref(pos).store(std::move(newPosition), std::memory_order_release);
    }

public:
    ThreadSafePosition(const SPACETIME &position) : pos(position) {}

    // agent read access need not be atomic
    inline const SPACETIME &position() const { return pos; }

    // tjread-safe position write with validity check for agent
    inline void jumpTo(const SPACETIME &position) {
        if(!(pos < position)) throw(std::runtime_error("An agent can only jumpTo a positions that are in its future light-cone"));
        setPosition(position);
    }

    // Threadsafe read for channel to read position
    inline SPACETIME getPosition() const { return std::atomic_ref(pos).load(std::memory_order_relaxed); }
};

#endif
