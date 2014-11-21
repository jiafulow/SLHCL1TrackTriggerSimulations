#ifndef AMSimulation_PatternMatcher_h_
#define AMSimulation_PatternMatcher_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTPattern.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/SuperstripArbiter.h"
using namespace slhcl1tt;

#include "fas/lean_lut3.h"
#include "fas/lean_merger3.h"
#include "fas/lean_counter3.h"

#define USE_LEAN_COUNTER


// SETTINGS: Majority logic
// INPUT   : TTree with moduleId, stub info, sim info + pattern bank
// OUTPUT  : Roads

class PatternMatcher {
  public:
    // Constructor
    PatternMatcher(PatternBankOption option)
    : po(option), nLayers_(po.nLayers), nDCBits_(po.nDCBits),
      prefixRoad_("AMTTRoads_"), suffix_(""),
      nEvents_(999999999), minFrequency_(1), maxPatterns_(999999999), maxRoads_(999999999), maxStubs_(999999999),
      verbose_(1),
      inputPatterns_fas_(0,0) {

        assert(3 <= nLayers_ && nLayers_ <= 8);
        assert(nDCBits_ <= 4);

        // Decide on the size of superstrip
        if (po.mode == 0)
            arbiter_ = new SuperstripArbiter(po.subLadderSize, po.subModuleSize);
        else
            arbiter_ = new SuperstripArbiter(po.subLadderVarSize, po.subModuleVarSize, po.subLadderECVarSize, po.subModuleECVarSize);
    }

    // Destructor
    ~PatternMatcher() {
        if (arbiter_)  delete arbiter_;
    }


    // Setters
    void setNEvents(long long n)    { if (n != -1)  nEvents_     = n > 0 ? n : 0; }
    void setMinFrequency(int n)     { minFrequency_ = n > 1 ? n : 1; }
    void setMaxPatterns(int n)      { if (n != -1)  maxPatterns_ = n > 0 ? n : 0; }
    void setMaxRoads(int n)         { if (n != -1)  maxRoads_    = n > 0 ? n : 0; }
    void setMaxStubs(int n)         { if (n != -1)  maxStubs_    = n > 0 ? n : 0; }
    void setVerbosity(int v)        { verbose_ = v; }

    // Getters
    // none

    // Functions
    int makeRoads_vector(TString src, TString bank, TString out);
    int makeRoads_fas(TString src, TString bank, TString out);

    // Main driver
    int run(TString src, TString bank, TString out);

  private:
    // Configurations
    const PatternBankOption po;
    const unsigned nLayers_;
    const unsigned nDCBits_;  // UNUSED
    const TString prefixRoad_;
    const TString suffix_;

    // Program options
    long long nEvents_;
    int minFrequency_;  // min frequency of patterns to be read out
    int maxPatterns_;   // max number of patterns
    int maxRoads_;      // max number of roads per event
    int maxStubs_;      // max number of stubs per superstrip
    int verbose_;

    // Operators
    SuperstripArbiter * arbiter_;

    // Pattern bank data
    std::vector<pattern_type> inputPatterns_vector_;  // using std::vector approach
    fas::lean_lut3 inputPatterns_fas_;                // using fas::lean_lut3 approach

    // Maps
    std::map<unsigned, std::vector<unsigned> > triggerTowerMap_;        // key: towerId, value: moduleIds in the tower
    std::map<unsigned, std::vector<unsigned> > triggerTowerReverseMap_; // key: moduleId, value: towerIds containing the module
};

#endif
