#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTRoad.h"
#include <iostream>

namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const TTRoad& road) {
    o << "Pattern no: " << road.patternRef << " tower: " << road.tower << " # stubs: " << road.nstubs << " superstripIds: (";
    for (unsigned i=0; i<road.superstripIds.size(); ++i)
        o << road.superstripIds.at(i) << ",";
    o << ")" << " # stubs/layer: (";
    for (unsigned i=0; i<road.stubRefs.size(); ++i)
        o << road.stubRefs.at(i).size() << ",";
    o << ")" << std::endl;
    return o;
}

}  // namespace slhcl1tt
