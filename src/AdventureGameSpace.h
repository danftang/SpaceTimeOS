#ifndef ADVENTUREGAMESPACE_H
#define ADVENTUREGAMESPACE_H

// AdventureGameSpace defines a space where the state of the positions in space are
// sets of predicates true of an agent at that point.
// Each agent has a unique ID, so binary predicates can be expressed by referring
// to an agent's ID.
// 
// TreeSpace:
// A treespace is a discrete space whose spatial positions are nodes in a tree.
// Each node is a 1D time dimension space.
// The children of a node are ordered/numbered, so a position in TreeSpace
// is an ordered list of integers of any length giving the child ID from
// the root node, and then a time on that node.
// We can define a distance metric as the square of the time difference minus the
// square of the length of the shortest path
// between points moving along parent-child connections.
// Velocity, then is either in the time direction,
// up to parent or towards a child.
// If time is integer, we can set the distance between nodes to sqrt(3)
// to ensure that unit velocities are possible between neighbouring nodes.
//
// [But one characteristic of a tree space is if an agent moves,
//  everything he is carrying goes with him. So in this way it really is
//  more of a binary relation space, where the parent/child relationship
//  is the only spatially important relationship. 
//  ...or we have a concept of a TreeSpace, but the way we encode position
//  is by parent pointers. So an agent really does move all the ojects
//  he carries, but this is represented in a computationally efficient manner.
//  ...or a pocket full of stuff is in-fact a factory of agents that are only
//  reified once we take them out of the pocket...and putting something into
//  the pocket destroys that object, but embues the pocket with the ability to
//  create that thing.]

class AdventureGameSpace {

};


#endif
