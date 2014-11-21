#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTPattern.h"
#include <iostream>


namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const pattern_type& patt) {
    for (pattern_type::const_iterator it=patt.begin(); it!=patt.end(); ++it) {
        o << *it << " ";
    }
    return o;
}

std::ostream& operator<<(std::ostream& o, const TTPattern& patt) {
    o << "id: " << patt.id << " freq: " << (unsigned) patt.frequency << " ";
    return o;
}

std::ostream& operator<<(std::ostream& o, const TTRoad& road) {
    o << " Bank index: " << road.bankIndex() << " triggerTowerId: " << road.triggerTowerId() << " # superstrips: " << (unsigned) road.nsuperstrips() << " # stubs: " << road.stubRefs().size() << " {(superstripId,index)}: ";
    for (unsigned i=0; i<road.stubRefs().size(); ++i) {
        o << "(" << road.stubSuperstripId(i) << "," << road.stubRef(i) << ")" << " ";
    }
    return o;
}

}  // namespace slhcl1tt
