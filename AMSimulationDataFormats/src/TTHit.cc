#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTHit.h"
#include <iostream>

namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const TTHit& hit) {
    o << "ref: " << hit.ref << " r: " << hit.r << " phi: " << hit.phi << " z: " << hit.z
      << " err_r: " << hit.rError << " err_phi: " << hit.phiError << " err_z: " << hit.zError
      << " ";
    return o;
}

std::ostream& operator<<(std::ostream& o, const std::vector<TTHit>& hits) {
    for (std::vector<TTHit>::const_iterator it=hits.begin(); it!=hits.end(); ++it) {
        o << *it << std::endl;
    }
    return o;
}

}  // namespace slhcl1tt
