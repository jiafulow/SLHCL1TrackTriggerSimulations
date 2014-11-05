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

std::ostream& operator<<(std::ostream& o, const std::vector<TTHit>& hits) {
    for (std::vector<TTHit>::const_iterator it=hits.begin(); it!=hits.end(); ++it) {
        o << *it << " ";
    }
    return o;
}

std::ostream& operator<<(std::ostream& o, const TTRoad& road) {
    o << "nSuperstrips: " << (unsigned) road.nSuperstrips() << " bank index: " << road.bankIndex() << " # TTHits: " << road.getHits().size() << " ";
    return o;
}

}  // namespace slhcl1tt
