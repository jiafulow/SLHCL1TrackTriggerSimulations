#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack2.h"
#include <iostream>


namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const TTTrack2& track) {
    o << "rinv: " << track.rinv() << " phi0: " << track.phi0() << " cottheta: " << track.cottheta() << " z0: " << track.z0() << " d0: " << track.d0() << " chi2: " << track.chi2() << " ndof: " << track.ndof() << " chi2_phi: " << track.chi2_phi() << " chi2_z: " << track.chi2_z()
      << " tpId: " << track.tpId()
      << " roadRef: " << track.roadRef() << " combRef: " << track.combRef() << " # stubs: " << track.stubRefs().size()
      << " ";
    return o;
}

}  // namespace slhcl1tt