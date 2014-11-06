#ifndef TTrackWriter_h_
#define TTrackWriter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"

#include "TString.h"
#include "TTree.h"
#include "TChain.h"
#include "TFile.h"
#include <memory>

namespace slhcl1tt{

class TTTrackWriter {
  public:
    TTTrackWriter(TChain *inChain, TString out, TString prefix, TString suffix);
    ~TTTrackWriter();

    void fill(const std::vector<TTTrack>& tracks); // const std::vector<GenParticle>& genParts);
    long long write();
    
  private:
    void init(TChain *inChain, TString out, TString prefix, TString suffix);

    TFile* tfile_;
    TTree* ttree_;

    // Tracks
    std::auto_ptr<std::vector<float> > tracks_rho_;
		std::auto_ptr<std::vector<float> > tracks_phi0_;
		std::auto_ptr<std::vector<float> > tracks_z0_;
		std::auto_ptr<std::vector<float> > tracks_cottheta0_;
		std::auto_ptr<std::vector<float> > tracks_chi2_;
		std::auto_ptr<std::vector<float> > tracks_chi2Red_;
    std::auto_ptr<std::vector<unsigned int> > tracks_roadIndex_;
    // Generator particles
    /*std::auto_ptr<std::vector<float> > genParts_pt_;
    std::auto_ptr<std::vector<float> > genParts_phi_;
    std::auto_ptr<std::vector<float> > genParts_eta_;
    std::auto_ptr<std::vector<float> > genParts_charge_;*/
};

} // namespace slhcl1tt

#endif
