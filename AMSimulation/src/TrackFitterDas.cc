#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterDas.h"

int stubUncertainty(double r, double &d_phi, double &d_z)
{
  if (fabs(r-22)<5 || fabs(r-34)<5 || fabs(r-50)<5) // PS Layers
  {
    d_phi=0.01/r;
    d_z=0.15;
  }
  else if (fabs(r-67)<5 || fabs(r-88)<5 || fabs(r-107)<5) // SS Layers
  {
    d_phi=0.009/r;
    d_z=5;
  }
  else
  {
    d_phi=-1;
    d_z=-1;
    return 0;
  }
  return 1;
} 
  

slhcl1tt::TTTrack* TrackFitterDas::fit(std::vector<float> &v_hitPhis, std::vector<float> &v_hitRs, std::vector<float> &v_hitZs)
{
  double A=0, B=0, C=0, D=0, E=0, F=0, G=0, H=0, I=0, J=0, K=0, L=0;
  for (unsigned int i=0; i<v_hitPhis.size(); ++i)
  {
    double r=v_hitRs.at(i);      
    double phi=v_hitPhis.at(i);  
    double z=v_hitZs.at(i);
    
    double d_phi=-1, d_z=-1;
    if (!stubUncertainty(r, d_phi, d_z)) {std::cout<<"ERROR: Unrecognized layer at r = "<<r<<std::endl; continue;}
    
    double d_phi_sq=d_phi*d_phi;
    double d_z_sq=d_z*d_z;
    
    A+=phi/d_phi_sq;                      
    B+=1/d_phi_sq;                        
    C+=r/d_phi_sq;                        
    D+=r*phi/d_phi_sq;                    
    E+=r*r/d_phi_sq;                      
    F+=r*z/d_z_sq;                      
    G+=r*r/d_z_sq;                      
    H+=r/d_z_sq;                        
    I+=z/d_z_sq;                        
    J+=1/d_z_sq;
    
    K+=phi*phi/d_phi_sq;
    L+=z*z/d_z_sq;                     
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
  
  track->chi2_phi_ = K + pow(track->phi0_, 2)*B + E/pow(2.*track->rho_ ,2)
                   - 2.*A*track->phi0_ + D/track->rho_ - (C*track->phi0_)/track->rho_;
                   
  track->chi2_z_   = L + G*pow(track->cottheta0_, 2) + J*pow(track->z0_, 2)
                   - 2.*F*track->cottheta0_ - 2.*I*track->z0_ + 2.*H*(track->z0_)*(track->cottheta0_);
                   
  return track;
}
