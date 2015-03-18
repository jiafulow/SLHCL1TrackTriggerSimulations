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
    int fit(const std::vector<TTHit>& hits, TTTrack2& track);
    
  private:  
    double computeParameter(int trackParameter, std::vector<TTHit>& dhits);
    unsigned int nStubCoordinates_;
    unsigned int nTrackParameters_;
    std::vector<std::vector<double> > m_V_, m_D_, m_VD_;
    std::vector<std::vector<double> > c_CentralStubCoords_Ladders_, c_CentralTrackParams_Ladders_;
    
};

#endif

