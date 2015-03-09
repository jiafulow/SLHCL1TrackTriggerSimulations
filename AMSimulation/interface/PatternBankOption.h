#ifndef AMSimulation_PatternBankOption_h_
#define AMSimulation_PatternBankOption_h_

#include <string>
#include <vector>
#include <iosfwd>

namespace slhcl1tt {

struct PatternBankOption {
    std::string superstrip;
    float minPt;
    float maxPt;
    float minEta;  // not absolute eta
    float maxEta;  // not absolute eta
    float minPhi;  // from -pi to pi
    float maxPhi;  // from -pi to pi
    unsigned nLayers;
    unsigned nFakers;
    unsigned nDCBits;
    unsigned tower;
    unsigned mode;
};

std::ostream& operator<<(std::ostream& o, const PatternBankOption& po);

}  // namespace slhcl1tt

#endif

