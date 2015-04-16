#ifndef AMSimulation_TrackFitterAlgoLinearized_h_
#define AMSimulation_TrackFitterAlgoLinearized_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
#include "LinearizedTrackFit/LinearizedTrackFit/interface/LinearizedTrackFitter.h"
using namespace slhcl1tt;

class TrackFitterAlgoLinearized : public TrackFitterAlgoBase
{
 public:
 TrackFitterAlgoLinearized() : TrackFitterAlgoBase() {}

  ~TrackFitterAlgoLinearized() {}

  virtual int fit(const std::vector<TTHit>& hits, TTTrack2& track);

 private:
  LinearizedTrackFitter linearizedTrackFitter_;
};

#endif

