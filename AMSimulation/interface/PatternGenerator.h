#ifndef AMSimulation_PatternGenerator_h_
#define AMSimulation_PatternGenerator_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Pattern.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/HelperMath.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
using namespace slhcl1tt;

#include "TFile.h"
#include "TFileCollection.h"
#include "TChain.h"
#include "TTree.h"
#include "TTreePlayer.h"
#include "TTreeFormula.h"
#include "TString.h"


// FIXME: incorporate trigger tower sectorization
// FIXME: fake hits
// FIXME: bank merging

// SETTINGS: DC bits, superstrip size, etc
// INPUT   : TTree with moduleId, hitId, sim info + trigger tower file
// OUTPUT  : TTree with active moduleId, and patterns

class PatternGenerator {
  public:
    //typedef std::map<uint64_t, std::shared_ptr<bool> > HitIdBoolMap;
    //typedef std::map<Pattern::pattern8_t, std::shared_ptr<bool> > PatternIdBoolMap;
    typedef std::map<uint64_t, std::shared_ptr<uint16_t> > HitIdShortMap;
    typedef std::map<Pattern::pattern8_t, std::shared_ptr<uint16_t> > PatternIdShortMap;
    typedef std::map<uint64_t, uint32_t> HitIdIndexMap;
    typedef std::map<Pattern::pattern8_t, uint32_t> PatternIdIndexMap;

    // Constructor
    PatternGenerator(PatternBankOption option)
    : po(option), nLayers_(po.nLayers), nDCBits_(po.nDCBits),
      filter_(true),
      nEvents_(999999999), nPatterns_(999999999),
      verbose_(1) {

        chain_ = new TChain("ntupler/tree");

        // Hardcoded layer information
        if (nLayers_ <= 6) {
            layerMap_ = std::map<uint32_t, uint32_t> {
                {5,5}, {6,6}, {7,7}, {8,8}, {9,9}, {10,10},
                {11,11}, {12,10}, {13,9}, {14,8}, {15,7},
                {18,11}, {19,10}, {20,9}, {21,8}, {22,7}
            };
        } else {  // otherwise it's not merged
            layerMap_ = std::map<uint32_t, uint32_t> {
                {5,5}, {6,6}, {7,7}, {8,8}, {9,9}, {10,10},
                {11,11}, {12,12}, {13,13}, {14,14}, {15,15},
                {18,11}, {19,12}, {20,13}, {21,14}, {22,15}
            };
        };

        eventSelect_ = "genParts_pt[0]>2 && abs(genParts_eta[0])<2.2 && Sum$(TTStubs_trkId==1 && abs(atan2(TTStubs_r,TTStubs_z)-atan2(genParts_pt[0],genParts_pz[0]))<0.05 && abs(deltaPhi(atan2(TTStubs_y,TTStubs_x),genParts_phi[0]))<0.03)>=Sum$(TTStubs_trkId==1)-2";
    }

    ~PatternGenerator() {}


    // Setters
    void setNLayers(int n)        { nLayers_ = n; }
    void setNDCBits(int n)        { nDCBits_ = n; }

    void setFilter(bool b=true)   { filter_ = b; }
    void setNEvents(int n)        { if (n != -1)  nEvents_ = std::max(0, n); }
    void setNPatterns(int n)      { if (n != -1)  nPatterns_ = std::max(0, n); }
    void setVerbosity(int n)      { verbose_ = n; }

    // Getters
    int getNLayers()        const { return nLayers_; }
    int getNDCBits()        const { return nDCBits_; }

    // Functions
    int readTriggerTowerFile(TString src);

    int readFile(TString src);

    int readAndFilterTree(TString out_tmp);  // make a temporary tree
    int readAndFilterTree2(TString out_tmp);  // make a temporary tree (testing)

    int makeTree(TString out_tmp);

    int writeTree(TString out);

    // Main driver
    int run(TString src, TString out, TString layout);

  private:
    void uniquifyPatterns();


  public:
    // Configurations
    const PatternBankOption po;

  private:
    // Configurations
    int nLayers_;
    int nDCBits_;

    // Program options
    bool filter_;
    int nEvents_;
    int nPatterns_;
    int verbose_;

    // Event selection
    TString eventSelect_;

    // Containers
    TChain * chain_;
    std::vector<Pattern> allPatterns_;
    std::vector<Pattern> goodPatterns_;

    //HitIdBoolMap hitIdMap_;           // key: hitId, value: pointer to a boolean
    HitIdShortMap hitIdMap_;          // key: hitId, value: pointer to a boolean

    std::map<uint32_t, uint32_t> layerMap_;  // defines layer merging
    std::map<uint32_t, Pattern::vuint32_t> triggerTowerMap_;
};

#endif
