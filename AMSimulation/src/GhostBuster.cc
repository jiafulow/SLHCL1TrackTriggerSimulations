#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/GhostBuster.h"
using namespace slhcl1tt;

#include <cassert>
#include <iostream>


// _____________________________________________________________________________
bool GhostBuster::isGhostRoad(const std::vector<unsigned>& superstripIds1, const std::vector<unsigned>& superstripIds2, const unsigned threshold) const {
    assert(superstripIds1.size() == superstripIds2.size());

    unsigned n = 0;
    for (unsigned i=0; i<superstripIds1.size(); ++i) {
        if (superstripIds1.at(i) == GhostBuster::BAD || superstripIds2.at(i) == GhostBuster::BAD)
            continue;

        if (superstripIds1.at(i) != superstripIds2.at(i)) {
            ++n;
            if (n >= threshold)
                return false;
        }
    }

    return true;
}

// _____________________________________________________________________________
bool GhostBuster::isGhostTrack(const std::vector<unsigned>& stubRefs1, const std::vector<unsigned>& stubRefs2, const unsigned threshold) const {
    assert(stubRefs1.size() == stubRefs2.size());

    unsigned n = 0;
    for (unsigned i=0; i<stubRefs1.size(); ++i) {
        if (stubRefs1.at(i) == GhostBuster::BAD || stubRefs2.at(i) == GhostBuster::BAD)
            continue;

        if (stubRefs1.at(i) != stubRefs2.at(i)) {
            ++n;
            if (n >= threshold)
               return false;
        }
    }

    return true;
}

// _____________________________________________________________________________
void GhostBuster::print() {
    std::cout << std::endl;
}
