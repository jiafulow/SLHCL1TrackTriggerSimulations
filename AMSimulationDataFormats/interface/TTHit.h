#ifndef AMSimulationDataFormats_TTHit_h_
#define AMSimulationDataFormats_TTHit_h_

#include <cmath>
#include <vector>
#include <iosfwd>

namespace slhcl1tt {

struct TTHit {
    unsigned ref;
    float r;
    float phi;
    float z;
    float rError;
    float phiError;
    float zError;

    float x() const { return r * std::cos(phi); }
    float y() const { return r * std::sin(phi); }
};


// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const TTHit& hit);

std::ostream& operator<<(std::ostream& o, const std::vector<TTHit>& hits);

}  // namespace slhcl1tt

#endif
