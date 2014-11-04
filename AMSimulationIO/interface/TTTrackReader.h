#ifndef AMSimulationIO_TTTrackReader_h_
#define AMSimulationIO_TTTrackReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack.h"

#include "TChain.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TROOT.h"
#include "TTree.h"
#include "TString.h"
#include <memory>
#include <vector>

namespace slhcl1tt {


// _____________________________________________________________________________
class TTTrackReader {
  public:
    TTTrackReader(int verbose=1);
    ~TTTrackReader();

    // FIXME: implement this

  private:
    TChain* tchain;
    int treenumber;
    const int verbose_;
};

// _____________________________________________________________________________
class TTTrackWriter {
  public:
    TTTrackWriter(int verbose=1);
    ~TTTrackWriter();

    int init(TString out, TString prefix, TString suffix);

    void fill(const std::vector<TTTrack>& tracks);

    Long64_t write();

  private:
    // Tracks
    std::auto_ptr<std::vector<float> >              vt_px;
    std::auto_ptr<std::vector<float> >              vt_py;
    std::auto_ptr<std::vector<float> >              vt_pz;
    std::auto_ptr<std::vector<float> >              vt_pt;
    std::auto_ptr<std::vector<float> >              vt_eta;
    std::auto_ptr<std::vector<float> >              vt_phi;
    std::auto_ptr<std::vector<float> >              vt_vx;
    std::auto_ptr<std::vector<float> >              vt_vy;
    std::auto_ptr<std::vector<float> >              vt_vz;
    std::auto_ptr<std::vector<float> >              vt_rinv;
    std::auto_ptr<std::vector<float> >              vt_chi2;
    std::auto_ptr<std::vector<float> >              vt_ptconsistency;
    std::auto_ptr<std::vector<unsigned> >           vt_nstubs;

    TFile* tfile;
    TTree* ttree;
    const int verbose_;
};

}  // namespace slhcl1tt

#endif