#ifndef AMSimulationDataFormats_TTSuperstrip_h_
#define AMSimulationDataFormats_TTSuperstrip_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"
#include <iosfwd>
#include <vector>

namespace slhcl1tt {

// TTSuperstrip is designed to be the building block of a TTPattern.
// Nominally, it corresponds to a physical region in the detector.
// A POD type is used to reduce memory consumption.
struct TTSuperstrip {
    id_type  id;
    bit_type bit;
};


// TTHit is designed to be the building block of a TTRoad.
// Nominally, it corresponds to the smallest unit of measurement in the detector.
// A POD type is used to reduce memory consumption.
struct TTHit {
    float r;
    float phi;
    float z;
    float rError;
    float phiError;
    float zError;
};


// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const TTSuperstrip& ss);

std::ostream& operator<<(std::ostream& o, const TTHit& hit);

std::ostream& operator<<(std::ostream& o, const std::vector<TTHit>& hits);

}  // namespace slhcl1tt

#endif
