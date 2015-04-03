#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoToyPCA.h"
#include "TFile.h"
#include <fstream>
#include <iostream>

std::string itoa(int i) 
{
  char res[10];
  sprintf(res, "%d", i);
  std::string ret(res);
  return ret;
}

int TrackFitterAlgoToyPCA::loadVD(std::string filename)
{
  std::ifstream inputFile;
  inputFile.open((filename).c_str());
  if (!inputFile)
  {
    std::cout<<"ERROR: Track fitting matrix file "<<filename<<" doesn't exist."<<std::endl;
    return 0;
  }
  
  // Read number of variables and number of track parameters
  inputFile >> nStubCoordinates_;
  inputFile >> nTrackParameters_;
  std::cout << "Number of stub coordinates = " << nStubCoordinates_ << std::endl;
  std::cout << "Number of track parameters = " << nTrackParameters_ << std::endl;
  std::cout << std::endl;
  
  double x;
  // Read eigenvalues
  for (unsigned int i=0; i<nStubCoordinates_; ++i)
  {
    inputFile>>x;
    v_sqrtEigenvalues_.push_back(x);
  }
  std::cout<<"Read sqrt(eigenvalues)"<<std::endl;
  
  // Read transformation matrix V
  for (unsigned int i=0; i<nStubCoordinates_; ++i)
  {
    std::vector<double> v_row;
    for (unsigned int j=0; j<nStubCoordinates_; ++j)
    {
      inputFile>>x;
      v_row.push_back(x);
    }
    m_V_.push_back(v_row);
  }
  std::cout<<"Read transformation matrix V"<<std::endl;
  
  // Read constants per ladder
  std::cout<<" Constants === "<<std::endl;
  for (int ladder=-1; ladder<77; ++ladder)
  {
    std::cout<<"Ladder = "<<ladder<<std::endl;
    std::vector<double> coordinates;
    if (ladder==-1) std::cout<<"Ladder central hit coordinates === "<<std::endl;
    for (unsigned int i=0; i<nStubCoordinates_; ++i)
    {
      inputFile>>x;
      coordinates.push_back(x);
      if (ladder==-1) std::cout<<x<<std::endl;
    }
    c_CentralStubCoords_Ladders_.push_back(coordinates);
    
    
    std::vector<double> params;
    if (ladder==-1) std::cout<<"Ladder central track parameters ==="<<std::endl;
    for (unsigned int i=0; i<nTrackParameters_; ++i)
    {
      inputFile>>x;
      params.push_back(x);
      if (ladder==-1) std::cout<<x<<std::endl;
    }
    c_CentralTrackParams_Ladders_.push_back(params);
    if (ladder==-1) std::cout<<"=== ==="<<std::endl;
  }
  std::cout<<"=== === "<<std::endl;
  
  // Read transformation matrix D
  for (unsigned int i=0; i<nTrackParameters_; ++i)
  {
    std::vector<double> row;
    for (unsigned int j=0; j<nStubCoordinates_; ++j)
    {
      inputFile>>x;
      row.push_back(x);
    }
    m_D_.push_back(row);
  }
  std::cout<<"Read transformation matrix D"<<std::endl;
  
  // Read VD
  std::cout<<"=== Matrix VD === "<<std::endl;
  for (unsigned int i=0; i<nTrackParameters_; ++i)
  {
    std::vector<double> row;
    for (unsigned int j=0; j<nStubCoordinates_; ++j)
    {
      inputFile>>x;
      row.push_back(x);
      std::cout<<x<<" ";
    }
    m_VD_.push_back(row);
    std::cout<<std::endl;
  }
  std::cout<<"=== ==="<<std::endl;
  
  // Initialize the v_h_principals_
  v_h_principals_.push_back(new TH1F("h_principal_0", "; Component 0", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_1", "; Component 1", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_2", "; Component 2", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_3", "; Component 3", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_4", "; Component 4", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_5", "; Component 5", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_6", "; Component 6", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_7", "; Component 7", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_8", "; Component 8", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_9", "; Component 9", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_10", "; Component 10", 100, -3., 3.));
  v_h_principals_.push_back(new TH1F("h_principal_11", "; Component 11", 100, -3., 3.));
  
  return 0;
}

double TrackFitterAlgoToyPCA::computeParameter(int trackParameter, std::vector<TTHit>& dhits)
{
  double param=0;
  for (unsigned int i=0; i<dhits.size(); ++i)
  {
    param = param + (m_VD_.at(trackParameter).at(i*2)) * (dhits.at(i).phi)
                  + (m_VD_.at(trackParameter).at(i*2+1)) * (dhits.at(i).z);
  }
  return param;
}

void TrackFitterAlgoToyPCA::computePrincipals(std::vector<TTHit>& dhits)
{
  // Multiplying Vh
  for (unsigned int i=0; i<dhits.size(); ++i)
  {
    double sum1=0;
    double sum2=0;
    for (unsigned int j=0; j<dhits.size(); ++j)
    {
      sum1 = sum1 + (m_V_.at(i*2).at(j*2))*(dhits.at(j).phi) + (m_V_.at(i*2).at(j*2+1))*(dhits.at(j).z);
      sum2 = sum2 + (m_V_.at(i*2+1).at(j*2))*(dhits.at(j).phi) + (m_V_.at(i*2+1).at(j*2+1))*(dhits.at(j).z);
    }
    v_h_principals_.at(i*2)->Fill(sum1/v_sqrtEigenvalues_.at(i*2));
    v_h_principals_.at(i*2+1)->Fill(sum2/v_sqrtEigenvalues_.at(i*2+1));
  }
  
  // Multiplying hV // this is wrong
  /*for (unsigned int i=0; i<dhits.size(); ++i) // columns
  {
    double sum1=0;
    double sum2=0;
    for (unsigned int j=0; j<dhits.size(); ++j) // rows
    {
      sum1 = sum1 + (m_V_.at(j*2).at(i*2))*(dhits.at(j).phi) + (m_V_.at(j*2+1).at(i*2))*(dhits.at(j).z);
      sum2 = sum2 + (m_V_.at(j*2).at(i*2+1))*(dhits.at(j).phi) + (m_V_.at(j*2+1).at(i*2+1))*(dhits.at(j).z);
    }
    v_h_principals_.at(i*2)->Fill(sum1/v_sqrtEigenvalues_.at(i*2));
    v_h_principals_.at(i*2+1)->Fill(sum2/v_sqrtEigenvalues_.at(i*2+1));
  }*/
  
}

int TrackFitterAlgoToyPCA::fit(const std::vector<TTHit>& hits, TTTrack2& track) 
{
   // Compute dhits;
   std::vector<TTHit> dhits;
   // std::cout<<"--- "<<std::endl;
   for (unsigned int i=0; i<hits.size(); ++i)
   {
     const TTHit& hit=hits.at(i);
     
     // These should be double
     float dphi=hit.phi-c_CentralStubCoords_Ladders_.at(0).at(i*2);
     float dz=hit.z-c_CentralStubCoords_Ladders_.at(0).at(i*2+1);
     
     dhits.emplace_back(TTHit{
                              0,
                              0,
                              dphi,
                              dz,
                              0.,
                              0.,
                              0.
                         });
     // std::cout<<"r = "<<hit.r<<std::endl;
   }
   
   computePrincipals(dhits);
   
   //Compute the track parameters
   double rinv      = c_CentralTrackParams_Ladders_.at(0).at(0)+computeParameter(0, dhits);
   double phi0      = c_CentralTrackParams_Ladders_.at(0).at(1)+computeParameter(1, dhits);
   double cottheta0 = c_CentralTrackParams_Ladders_.at(0).at(2)+computeParameter(2, dhits);
   double z0        = c_CentralTrackParams_Ladders_.at(0).at(3)+computeParameter(3, dhits);
   
   int ndof = (hits.size());
   
   track.setTrackParams(rinv, phi0, cottheta0, z0, 0, 0, ndof, 0, 0);

   return 0;
}

void TrackFitterAlgoToyPCA::savePlots()
{
  TFile *file=new TFile("Principals.root", "RECREATE");
  for (unsigned int i=0; i<v_h_principals_.size(); ++i)
  {
    v_h_principals_.at(i)->Write();
  }
  file->Write();
  file->Close();
}

