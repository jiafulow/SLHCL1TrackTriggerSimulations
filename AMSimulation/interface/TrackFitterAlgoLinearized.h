#ifndef AMSimulation_TrackFitterAlgoLinearized_h_
#define AMSimulation_TrackFitterAlgoLinearized_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
using namespace slhcl1tt;


class TrackFitterAlgoLinearized : public TrackFitterAlgoBase {
  public:
    TrackFitterAlgoLinearized() : TrackFitterAlgoBase() {}

    ~TrackFitterAlgoLinearized() {}

    int fit(const std::vector<TTHit>& hits, TTTrack2& track);
};

#endif

