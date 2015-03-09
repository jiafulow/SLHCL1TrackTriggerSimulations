#ifndef AMSimulationDataFormats_TTRoad_h_
#define AMSimulationDataFormats_TTRoad_h_

#include <vector>
#include <iosfwd>


namespace slhcl1tt {

struct TTRoad {
    unsigned patternRef;
    unsigned tower;
    unsigned nstubs;
    std::vector<unsigned> superstripIds;  // necessary?
    std::vector<std::vector<unsigned> > stubRefs;  // stubRefs[superstrip i][stub j]

    void reset() {
        patternRef  = 0;
        tower       = 99;
        nstubs      = 0;

        superstripIds.clear();
        stubRefs.clear();

        superstripIds.resize(8);
        stubRefs.resize(8);
    }
};


// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const TTRoad& road);

}  // namespace slhcl1tt

#endif
