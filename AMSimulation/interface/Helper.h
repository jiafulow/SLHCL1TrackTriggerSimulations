#ifndef AMSimulation_Helper_h_
#define AMSimulation_Helper_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"


namespace slhcl1tt {

/// Functions

// Round to half-strip precision
inline unsigned halfStripRound(float x) {
    static const float p = 10.;
    return floor((x*2)*p + 0.5)/p;
}

// Retrieve layer, ladder, module from a moduleId
inline unsigned decodeLayer(unsigned moduleId) {
    return (moduleId / 10000);
}

inline unsigned decodeLadder(unsigned moduleId) {
    return (moduleId / 100) % 100;
}

inline unsigned decodeModule(unsigned moduleId) {
    return (moduleId) % 100;
}

inline bool isPSModule(unsigned moduleId) {
    unsigned lay = decodeLayer(moduleId);
    if (5 <= lay && lay <= 7)
        return true;
    unsigned lad = decodeLadder(moduleId);
    if (11 <= lay && lay <= 22 && lad <= 8)
        return true;
    return false;
}

inline bool isBarrelModule(unsigned moduleId) {
    unsigned lay = decodeLayer(moduleId);
    if (5 <= lay && lay <= 10)
        return true;
    return false;
}

inline unsigned encodeModuleId(unsigned lay, unsigned lad, unsigned mod) {
    return (10000*lay + 100*lad + mod);
}

// Compress layer id
inline unsigned compressLayer(const unsigned& lay) {
    if (lay <  5) return 255;
    if (lay < 16) return lay-5;  // 5-10 = barrel, 11-15 = endcap +
    if (lay < 18) return 255;
    if (lay < 23) return lay-7;  // 18-22 = endcap -
    if (lay < 25) return 255;
    if (lay < 28) return lay-9;  // 25 = calo, 26 = muon, 27 = fake
    return 255;
}

}  // namespace slhcl1tt

#endif

