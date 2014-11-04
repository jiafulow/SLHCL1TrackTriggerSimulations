#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"

#include <iostream>
#include <iterator>


namespace slhcl1tt {

std::ostream& operator<<(std::ostream& o, const PatternBankOption& po) {
    o << "minPt: " << po.minPt << "  maxPt: " << po.maxPt << "  minEta: " << po.minEta << "  maxEta: " << po.maxEta << "  minPhi: " << po.minPhi << "  maxPhi: " << po.maxPhi << std::endl;
    o << "unitPhi: " << po.unitPhi << "  unitZ: " << po.unitZ << "  subLadderSize: " << po.subLadderSize << "  subModuleSize: " << po.subModuleSize << "  nLayers: " << po.nLayers << "  nMisses: " << po.nMisses << "  nFakers: " << po.nFakers << "  nDCBits: " << po.nDCBits << "  mode: " << po.mode << "  requireTriggerTower: " << po.requireTriggerTower << std::endl;
    o << "subLadderVarSize: ";
    std::copy(po.subLadderVarSize.begin(), po.subLadderVarSize.end(), std::ostream_iterator<unsigned>(o, " "));
    o << std::endl;
    o << "subModuleVarSize: ";
    std::copy(po.subModuleVarSize.begin(), po.subModuleVarSize.end(), std::ostream_iterator<unsigned>(o, " "));
    o << std::endl;
    o << "subLadderECVarSize: ";
    std::copy(po.subLadderECVarSize.begin(), po.subLadderECVarSize.end(), std::ostream_iterator<unsigned>(o, " "));
    o << std::endl;
    o << "subModuleECVarSize: ";
    std::copy(po.subModuleECVarSize.begin(), po.subModuleECVarSize.end(), std::ostream_iterator<unsigned>(o, " "));
    o << std::endl;
    o << "triggerTowers: ";
    std::copy(po.triggerTowers.begin(), po.triggerTowers.end(), std::ostream_iterator<unsigned>(o, " "));
    o << std::endl;
    return o;
}

}
