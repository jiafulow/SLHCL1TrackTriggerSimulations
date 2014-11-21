#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterOption.h"

#include <iostream>
#include <iterator>


namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const TrackFitterOption& po) {
    o << "maxChi2Red: " << po.maxChi2Red << "  minNdof: " << po.minNdof << "  mode: " << po.mode << std::endl;
    return o;
}

}
