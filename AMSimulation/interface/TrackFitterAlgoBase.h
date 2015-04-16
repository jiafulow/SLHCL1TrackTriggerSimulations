#ifndef AMSimulation_TrackFitterAlgoBase_h_
#define AMSimulation_TrackFitterAlgoBase_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTHit.h"
//#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack2.h"
using namespace slhcl1tt;

#include <iosfwd>
#include <vector>


class TrackFitterAlgoBase {
  public:
    TrackFitterAlgoBase() {}
    virtual ~TrackFitterAlgoBase() {}

    virtual int fit(const std::vector<TTHit>& hits, TTTrack2& track) { return 0; }
    virtual int fit(const std::vector<TTHit>& hits, std::vector<TTTrack2>& track, unsigned tower, unsigned roadRef) { return 0; }

};

#endif

