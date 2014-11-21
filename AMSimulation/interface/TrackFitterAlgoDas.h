#ifndef AMSimulation_TrackFitterAlgoDas_h_
#define AMSimulation_TrackFitterAlgoDas_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
using namespace slhcl1tt;


class TrackFitterAlgoDas : public TrackFitterAlgoBase {
  public:
    TrackFitterAlgoDas() : TrackFitterAlgoBase() {}

    ~TrackFitterAlgoDas() {}

    int fit(const std::vector<TTHit>& hits, TTTrack2& track);
};

#endif
