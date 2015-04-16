#ifndef AMSimulation_TrackFitterAlgoToyPCA_h_
#define AMSimulation_TrackFitterAlgoToyPCA_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
#include "TH1F.h"
#include <string>

using namespace slhcl1tt;

class TrackFitterAlgoToyPCA : public TrackFitterAlgoBase
{
  public:
  TrackFitterAlgoToyPCA(const std::string & filename);
  virtual ~TrackFitterAlgoToyPCA();

  // int loadVD(std::string filename);
    void   computePrincipals(std::vector<TTHit>& dhits);
    double computePrincipal(int i, std::vector<TTHit>& dhits);
    virtual int fit(const std::vector<TTHit>& hits, TTTrack2& track);
    // void savePlots();
    
  private:  
    double computeParameter(int trackParameter, std::vector<TTHit>& dhits);
    unsigned int nStubCoordinates_;
    unsigned int nTrackParameters_;
    std::vector<double> v_sqrtEigenvalues_;
    std::vector<std::vector<double> > m_V_, m_D_, m_VD_;
    std::vector<std::vector<double> > c_CentralStubCoords_Ladders_, c_CentralTrackParams_Ladders_;
    std::vector<TH1F*> v_h_principals_;
    
};

#endif

