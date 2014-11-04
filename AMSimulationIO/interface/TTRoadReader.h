#ifndef AMSimulationIO_TTRoadReader_h_
#define AMSimulationIO_TTRoadReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTPattern.h"

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
class TTRoadReader {
  public:
    TTRoadReader(int verbose=1);
    ~TTRoadReader();

    int init(TString src, TString prefix, TString suffix);

    Long64_t loadTree(Long64_t entry);

    Int_t getEntry(Long64_t entry);

    TChain* getChain() { return tchain; }

    // Roads
    std::vector<count_type> *               vr_nHitLayers;
    std::vector<id_type> *                  vr_bankIndex;
    //
    std::vector<std::vector<float> > *      vr_hitRs;
    std::vector<std::vector<float> > *      vr_hitPhis;
    std::vector<std::vector<float> > *      vr_hitZs;
    std::vector<std::vector<float> > *      vr_hitRErrors;
    std::vector<std::vector<float> > *      vr_hitPhiErrors;
    std::vector<std::vector<float> > *      vr_hitZErrors;
    std::vector<std::vector<int> > *        vr_hitCharges;
    std::vector<std::vector<float> > *      vr_hitPts;
    std::vector<std::vector<id_type> > *    vr_hitSuperstripIds;
    std::vector<std::vector<int> > *        vr_hitTrkIds;

    // genParticle info
    std::vector<float> * vp_pt;
    std::vector<float> * vp_eta;
    std::vector<float> * vp_phi;
    std::vector<float> * vp_vx;
    std::vector<float> * vp_vy;
    std::vector<float> * vp_vz;
    std::vector<int> *   vp_charge;

  private:
    TChain* tchain;
    int treenumber;
    const int verbose_;
};


// _____________________________________________________________________________
class TTRoadWriter {
  public:
    TTRoadWriter(int verbose=1);
    ~TTRoadWriter();

    int init(TString out, TString prefix, TString suffix);

    void fill(const std::vector<TTRoad>& roads, const std::vector<GenParticle>& genParts);

    Long64_t write();

  private:
    // Roads
    std::auto_ptr<std::vector<count_type> >               vr_nHitLayers;
    std::auto_ptr<std::vector<id_type> >                  vr_bankIndex;
    //
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitRs;
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitPhis;
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitZs;
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitRErrors;
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitPhiErrors;
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitZErrors;
    std::auto_ptr<std::vector<std::vector<int> > >        vr_hitCharges;
    std::auto_ptr<std::vector<std::vector<float> > >      vr_hitPts;
    std::auto_ptr<std::vector<std::vector<id_type> > >    vr_hitSuperstripIds;
    std::auto_ptr<std::vector<std::vector<int> > >        vr_hitTrkIds;

    // Roads (inner vectors)
    std::vector<float>      hitRs;
    std::vector<float>      hitPhis;
    std::vector<float>      hitZs;
    std::vector<float>      hitRErrors;
    std::vector<float>      hitPhiErrors;
    std::vector<float>      hitZErrors;
    std::vector<int>        hitCharges;
    std::vector<float>      hitPts;
    std::vector<id_type>    hitSuperstripIds;
    std::vector<int>        hitTrkIds;

    // In addition, keep genParticle info
    std::auto_ptr<std::vector<float> > vp_pt;
    std::auto_ptr<std::vector<float> > vp_eta;
    std::auto_ptr<std::vector<float> > vp_phi;
    std::auto_ptr<std::vector<float> > vp_vx;
    std::auto_ptr<std::vector<float> > vp_vy;
    std::auto_ptr<std::vector<float> > vp_vz;
    std::auto_ptr<std::vector<int> >   vp_charge;

    TFile* tfile;
    TTree* ttree;
    const int verbose_;
};

}  // namespace slhcl1tt

#endif
