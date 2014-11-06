#ifndef TrackFitterDas_h_
#define TrackFitterDas_h_

#include "TrackFitterAlgo.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack.h"

class TrackFitterDas : TrackFitterAlgoBase
{
  public:
	  slhcl1tt::TTTrack* fit(std::vector<float> &v_hitPhis, std::vector<float> &v_hitRs, std::vector<float> &v_hitZs);
};
	  
#endif
