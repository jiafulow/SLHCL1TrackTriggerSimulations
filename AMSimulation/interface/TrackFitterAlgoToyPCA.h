#ifndef AMSimulation_TrackFitterAlgoToyPCA_h_
#define AMSimulation_TrackFitterAlgoToyPCA_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
#include <string>

using namespace slhcl1tt;

class TrackFitterAlgoToyPCA
{
  public:
    TrackFitterAlgoToyPCA(){}
    ~TrackFitterAlgoToyPCA() {}

    int loadVD(std::string filename);
    int fit(const std::vector<TTHit>& hits, const std::vector<TTHit>& centralHits, TTTrack2& track);
    
  private:  
    double computeParameter(int trackParameter, std::vector<TTHit>& dhits);
    std::vector<std::vector<double> > m_VD_;
    
};

#endif

