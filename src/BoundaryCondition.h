#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

// A BoundaryCondition of a given simulation defines a ``simulation frontier'' which is a volume
// in spacetime that defines the extent of the simulation. 
// This object allows the user to spawn agents in any position outside of the simulated frontier,
// create arbitrary channels between them and extend the frontier.
// All this can be in the simulation.
template<class SPACETIME>
class BoundaryCondition {
public:
    typedef SPACETIME SpaceTime;

    bool isInBounds(const SpaceTime &pos);

    void spawnAt(); // new agetns must be out of current bounds. New agents are started when they enter the bounds of the simulation.
    // deletion of agents: An agent deletes itself when:
    //   - it has no input channels and its loop channel is empty
    //   - when it reaches the simulation boundary it can check the state of the simulation to decide whether to delete itself or
    //     submit its callback to the simulation.
    // 
    // 
    template<class T> connect(source, target); // new agents can connect to agents in the simulation, but agents in the sim must 
    // connections can be set up between any newly created agent. [the source always initiates new connections]
};

#endif
