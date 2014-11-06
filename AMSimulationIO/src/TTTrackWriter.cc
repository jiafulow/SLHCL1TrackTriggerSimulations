#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackWriter.h"

using namespace slhcl1tt;

TTTrackWriter::TTTrackWriter(TChain *inChain, TString out, TString prefix, TString suffix)
: tracks_rho_         (new std::vector<float>()),
  tracks_phi0_  			(new std::vector<float>()),
  tracks_z0_  				(new std::vector<float>()),
  tracks_cottheta0_ 	(new std::vector<float>()),
  tracks_chi2_        (new std::vector<float>()),
  tracks_chi2Red_ 		(new std::vector<float>()),
  tracks_roadIndex_   (new std::vector<unsigned int>())
  /*,
  genParts_pt_        (new std::vector<float>()),
  genParts_phi_       (new std::vector<float>()),
  genParts_eta_       (new std::vector<float>()),
  genParts_charge_    (new std::vector<float>())*/
{
  init(inChain, out, prefix, suffix);
}

TTTrackWriter::~TTTrackWriter() 
{
  if (ttree_)  delete ttree_;
  if (tfile_)  delete tfile_;
}

void TTTrackWriter::init(TChain *inChain, TString out, TString prefix, TString suffix) 
{
  if (!out.EndsWith(".root")) throw std::invalid_argument("Output filename must be .root.");
  tfile_ = TFile::Open(out, "RECREATE");
  if (tfile_ == 0) throw std::runtime_error("Cannot recreate output file.");
  tfile_->mkdir("ntupler")->cd();
  
  //ttree_ = new TTree("tree", "");
  ttree_=inChain->CloneTree(0);

  ttree_->Branch(prefix + "rho"       + suffix, &(*tracks_rho_));
  ttree_->Branch(prefix + "phi0"      + suffix, &(*tracks_phi0_ ));
  ttree_->Branch(prefix + "z0"        + suffix, &(*tracks_z0_));
  ttree_->Branch(prefix + "cottheta0" + suffix, &(*tracks_cottheta0_));
  ttree_->Branch(prefix + "chi2"      + suffix, &(*tracks_chi2_));
  ttree_->Branch(prefix + "chi2Red"   + suffix, &(*tracks_chi2Red_));
  ttree_->Branch(prefix + "roadIndex" + suffix, &(*tracks_roadIndex_));
  // ttree_->Branch("genParts_pt",     &(*genParts_pt_));
  // ttree_->Branch("genParts_eta",    &(*genParts_eta_));
  // ttree_->Branch("genParts_phi",    &(*genParts_phi_));
  // ttree_->Branch("genParts_charge", &(*genParts_charge_));
}

void TTTrackWriter::fill(const std::vector<TTTrack>& tracks) //, const std::vector<GenParticle>& genParts)
{
  tracks_rho_       ->clear();
  tracks_phi0_      ->clear();
  tracks_z0_        ->clear();
  tracks_cottheta0_ ->clear();
  tracks_chi2_      ->clear();
  tracks_chi2Red_   ->clear();
  tracks_roadIndex_ ->clear();
 
  /*genParts_pt_     ->clear();
  genParts_phi_    ->clear();
  genParts_eta_    ->clear();
  genParts_charge_ ->clear();*/
  
  const unsigned int ntracks=tracks.size();
  for (unsigned int i=0; i<ntracks; ++i)
  {
    const TTTrack& track=tracks.at(i);
    
    tracks_rho_         ->push_back(track.rho_);  
    tracks_phi0_        ->push_back(track.phi0_); 
    tracks_z0_          ->push_back(track.z0_);   
    tracks_cottheta0_   ->push_back(track.cottheta0_);
    tracks_chi2_        ->push_back(track.chi2_);
    tracks_chi2Red_     ->push_back(track.chi2Red_);
    tracks_roadIndex_   ->push_back(track.getRoadIndex());
  }
  
  /*const unsigned int nparts=genParts.size();
  for (unsigned int i=0; i<nparts; ++i)
  {
    const GenParticle& part=genParts.at(i);
    
    genParts_pt_     ->push_back(part.pt);
    genParts_phi_    ->push_back(part.phi);
    genParts_eta_    ->push_back(part.eta);
    genParts_charge_ ->push_back(part.charge);
  }*/
  
  ttree_->Fill();
  assert(tracks_rho_->size()==ntracks);
  // assert(genParts_pt_->size()==nparts);
}

long long TTTrackWriter::write()
{
  long long nentries=ttree_->GetEntries();
  tfile_->Write();
  return nentries;
}  
