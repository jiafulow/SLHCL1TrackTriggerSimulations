#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoToyPCA.h"
#include <fstream>
#include <iostream>

int TrackFitterAlgoToyPCA::loadVD(std::string filename)
{
  std::ifstream file;
  file.open(filename.c_str());
  if (!file)
  {
    std::cout<<"ERROR: Track fitting matrix file "<<filename<<" doesn't exist."<<std::endl;
    return 0;
  }
  
  double x;
  for (unsigned int i=0; i<4; ++i) // Track parameters 0 - 3
  {
    std::vector<double> v_row;
    for (unsigned int j=0; j<12; ++j) // Hit coordinates 0 - 11
    {
      file>>x;
      v_row.push_back(x);
    }
    m_VD_.push_back(v_row);
  }
  return 0;
}

double TrackFitterAlgoToyPCA::computeParameter(int trackParameter, std::vector<TTHit>& dhits)
{
  double param=0;
  for (unsigned int i=0; i<dhits.size(); ++i)
  {
    param = param + (m_VD_.at(trackParameter).at(i*3)) * (dhits.at(i).r)
                  + (m_VD_.at(trackParameter).at(i*3+1)) * (dhits.at(i).phi)
                  + (m_VD_.at(trackParameter).at(i*3+2)) * (dhits.at(i).z);
  }
  return param;
}

int TrackFitterAlgoToyPCA::fit(const std::vector<TTHit>& hits, const std::vector<TTHit>& centralHits, TTTrack2& track) 
{
   // Compute dhits;
   std::vector<TTHit> dhits;
   for (unsigned int i=0; i<hits.size(); ++i)
   {
     const TTHit& hit=hits.at(i);
     const TTHit& centralHit=centralHits.at(i);
     
     // These should be double
     float dr=hit.r-centralHit.r;
     float dphi=hit.phi-centralHit.phi;
     float dz=hit.z-centralHit.z;
     
     dhits.emplace_back(TTHit{
                              hit.ref,
                              dr,
                              dphi,
                              dz,
                              0.,
                              0.,
                              0.
                         });
   }
   
   //Compute the track parameters
   double rinv      = computeParameter(1, dhits);
   double phi0      = computeParameter(2, dhits);
   double z0        = computeParameter(3, dhits);
   double cottheta0 = computeParameter(4, dhits);
   
   int ndof = (hits.size());
   
   track.setTrackParams(rinv, phi0, cottheta0, z0, 0, 0, ndof, 0, 0);

   return 0;
}

