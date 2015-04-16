#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoLinearized.h"


int TrackFitterAlgoLinearized::fit(const std::vector<TTHit>& hits, TTTrack2& track) 
{
  // Arrange the stub coordinates in the format expected by the fitter
  std::vector<double> vars;
  for (unsigned int i=0; i<hits.size(); ++i) {
    const TTHit& hit=hits.at(i);
    vars.push_back(hit.phi);
    vars.push_back(hit.r);
    vars.push_back(hit.z);
  }
  
  double normChi2 = linearizedTrackFitter_.fit(vars);
  std::vector<double> pars(linearizedTrackFitter_.estimatedPars());
  // TODO: This should be returned by the LinearizedTrackFitter
  int nDof = 8;
  
  track.setTrackParams(pars[0], pars[1], pars[2], pars[3], 0, normChi2*nDof, nDof, 0, 0);
  
  return 0;
}

