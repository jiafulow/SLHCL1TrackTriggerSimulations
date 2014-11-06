#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterDas.h"

slhcl1tt::TTTrack* TrackFitterDas::fit(std::vector<float> &v_hitPhis, std::vector<float> &v_hitRs, std::vector<float> &v_hitZs)
{
  double A=0, B=0, C=0, D=0, E=0, F=0, G=0, H=0, I=0, J=0, K=0, L=0;
  for (unsigned int i=0; i<v_hitPhis.size(); ++i)
  {
    double r=v_hitRs.at(i);      
    double phi=v_hitPhis.at(i);  
    double z=v_hitZs.at(i);      
    A+=phi;                      
    B+=1;                        
    C+=r;                        
    D+=r*phi;                    
    E+=r*r;                      
    F+=r*z;                      
    G+=r*r;                      
    H+=r;                        
    I+=z;                        
    J+=1;
    
    K+=phi*phi;
    L+=z*z;                        
  }
  slhcl1tt::TTTrack *track=new slhcl1tt::TTTrack();
  track->rho_=0.5*(C*C-E*B)/(B*D-A*C);
  track->phi0_=(D*C-A*E)/(C*C-E*B);
  track->z0_=(F*H-I*G)/(H*H-J*G);
  track->cottheta0_=(F*J-H*I)/(G*J-H*H);
  track->chi2_ = K + pow(track->phi0_, 2)*B + E/pow(2.*track->rho_ ,2)
               - 2.*A*track->phi0_ + D/track->rho_ - (C*track->phi0_)/track->rho_
               + L + G*pow(track->cottheta0_, 2) + J*pow(track->z0_, 2)
               - 2.*F*track->cottheta0_ - 2.*I*track->z0_ + 2.*H*(track->z0_)*(track->cottheta0_);
  track->chi2Red_ = (track->chi2_)/double(v_hitPhis.size()-4);
  return track;
}
