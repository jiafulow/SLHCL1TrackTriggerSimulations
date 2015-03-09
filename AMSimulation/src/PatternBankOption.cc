#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"

#include <iostream>
#include <iterator>


namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const PatternBankOption& po) {
    o << "superstrip: " << po.superstrip << "  minPt: " << po.minPt << "  maxPt: " << po.maxPt << "  minEta: " << po.minEta << "  maxEta: " << po.maxEta << "  minPhi: " << po.minPhi << "  maxPhi: " << po.maxPhi << std::endl;
    o << "nLayers: " << po.nLayers << "  nFakers: " << po.nFakers << "  nDCBits: " << po.nDCBits << "  tower: " << po.tower << "  mode: " << po.mode << std::endl;
    return o;
}

}
