#ifndef AMSimulationIO_BasicReader_h_
#define AMSimulationIO_BasicReader_h_

#include "TChain.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "TROOT.h"
#include "TString.h"
#include "TTree.h"
#include <memory>
#include <vector>

namespace slhcl1tt {


// _____________________________________________________________________________
class BasicReader {
  public:
    BasicReader(int verbose=1);
    ~BasicReader();

    int init(TString src, bool full=true);

    Long64_t loadTree(Long64_t entry) { return tchain->LoadTree(entry); }

    Int_t getEntry(Long64_t entry) { return tchain->GetEntry(entry); }

    TChain* getChain() { return tchain; }

    // genParticle information
    std::vector<float> *          vp_pt;
    std::vector<float> *          vp_eta;
    std::vector<float> *          vp_phi;
    std::vector<float> *          vp_vx;
    std::vector<float> *          vp_vy;
    std::vector<float> *          vp_vz;
    std::vector<int> *            vp_charge;

    // Stub information
    std::vector<float> *          vb_x;
    std::vector<float> *          vb_y;
    std::vector<float> *          vb_z;
    std::vector<float> *          vb_r;
    std::vector<float> *          vb_eta;
    std::vector<float> *          vb_phi;
    std::vector<float> *          vb_coordx;
    std::vector<float> *          vb_coordy;
    std::vector<float> *          vb_roughPt;
    std::vector<float> *          vb_trigBend;
    std::vector<float> *          vb_clusWidth;
    std::vector<unsigned> *       vb_modId;
    std::vector<int> *            vb_tpId;

  protected:
    TChain* tchain;
    int treenumber;
    const int verbose_;
};


// _____________________________________________________________________________
class BasicWriter {
  public:
    BasicWriter(int verbose=1);
    ~BasicWriter();

    int init(TChain* tchain, TString out);

    Long64_t writeTree();

  protected:
    TFile* tfile;
    TTree* ttree;
    const int verbose_;
};

}  // namespace slhcl1tt

#endif
