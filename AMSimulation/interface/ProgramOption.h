#ifndef AMSimulation_ProgramOption_h_
#define AMSimulation_ProgramOption_h_

#include <string>
#include <vector>
#include <iosfwd>

namespace slhcl1tt {

struct ProgramOption {
    std::string input;
    std::string output;
    std::string bankfile;
    std::string matrixfile;
    std::string roadfile;
    std::string trackfile;
    std::string htmfile;

    int         verbose;
    int         speedup;
    long long   maxEvents;

    unsigned    nLayers;
    unsigned    nFakers;
    unsigned    nDCBits;

    unsigned    tower;
    std::string superstrip;
    std::string algo;
    std::string htmconf;

    float       minPt;
    float       maxPt;
    float       minInvPt;
    float       maxInvPt;
    float       minEta;
    float       maxEta;
    float       minPhi;
    float       maxPhi;
    float       minVz;
    float       maxVz;

    int         picky;
    int         minFrequency;
    long int    maxPatterns;
    int         maxMisses;
    int         maxStubs;
    int         maxRoads;

    std::string view;
    unsigned    hitBits;

    float       maxChi2;
    bool        CutPrincipals;
    int         minNdof;
    int         maxCombs;
    int         maxTracks;

    int         rmDuplicate;
    bool        rmParDuplicate;

    bool        oldCB;
    bool        FiveOfSix;
    bool        PDDS;

    bool        no_trim;
    bool        removeOverlap;

    std::string datadir;
};

std::ostream& operator<<(std::ostream& o, const ProgramOption& po);

}  // namespace slhcl1tt

#endif

