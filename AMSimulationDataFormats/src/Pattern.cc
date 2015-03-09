#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Pattern.h"
#include <iostream>
#include <iomanip>

namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const pattern_type& patt) {
    for (pattern_type::const_iterator it=patt.begin(); it!=patt.end(); ++it) {
        o << std::setw(6) << *it << " ";
    }
    return o;
}

std::ostream& operator<<(std::ostream& o, const pattern_bit_type& patt) {
    for (pattern_bit_type::const_iterator it=patt.begin(); it!=patt.end(); ++it) {
        o << *it << " ";
    }
    return o;
}

}  // namespace slhcl1tt
