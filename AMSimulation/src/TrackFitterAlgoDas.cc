#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoDas.h"
#include <iostream>


int stubUncertainty(double r, double &d_phi, double &d_z)
{
  if (fabs(r-22)<5 || fabs(r-34)<5 || fabs(r-50)<5) // PS Layers
  {
    d_phi=0.01/r;
    d_z=0.15;
  }
  else if (fabs(r-67)<5 || fabs(r-88)<5 || fabs(r-107)<5) // 2S Layers
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

int TrackFitterAlgoDas::fit(const std::vector<TTHit>& hits, TTTrack2& track)
{
  double A=0, B=0, C=0, D=0, E=0, F=0, G=0, H=0, I=0, J=0, K=0, L=0;
  for (unsigned int i=0; i<hits.size(); ++i)
  {
    const TTHit& tthit = hits.at(i);
    double r=tthit.r;
    double phi=tthit.phi;
    double z=tthit.z;

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

  double rho=0.5*(C*C-E*B)/(B*D-A*C);
  double phi0=(D*C-A*E)/(C*C-E*B);
  double z0=(F*H-I*G)/(H*H-J*G);
  double d0=0.;  // dummy
  double cottheta0=(F*J-H*I)/(G*J-H*H);
  double chi2  = K + pow(phi0, 2)*B + E/pow(2.*rho, 2)
               - 2.*A*phi0 + D/rho - (C*phi0)/rho
               + L + G*pow(cottheta0, 2) + J*pow(z0, 2)
               - 2.*F*cottheta0 - 2.*I*z0 + 2.*H*(z0)*(cottheta0);
  int    ndof  = (hits.size()-4);

  double chi2_phi  = K + pow(phi0, 2)*B + E/pow(2.*rho, 2)
                   - 2.*A*phi0 + D/rho - (C*phi0)/rho;

  double chi2_z    = L + G*pow(cottheta0, 2) + J*pow(z0, 2)
                   - 2.*F*cottheta0 - 2.*I*z0 + 2.*H*(z0)*(cottheta0);

  track.setTrackParams(rho, phi0, cottheta0, z0, d0, chi2, ndof, chi2_phi, chi2_z);

  return 0;
}
