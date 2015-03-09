#ifndef AMSimulation_PatternMatcher_h_
#define AMSimulation_PatternMatcher_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Pattern.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TriggerTowerMap.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/SuperstripArbiter.h"
using namespace slhcl1tt;


class PatternMatcher {
  public:
    // Constructor
    PatternMatcher(PatternBankOption po)
    : po_(po),
      prefixRoad_("AMTTRoads_"), suffix_(""),
      nEvents_(999999999), minFrequency_(1), maxPatterns_(999999999), maxMisses_(0),
      maxStubs_(999999999), maxRoads_(999999999), verbose_(1) {

        // Initialize
        ttmap_   = new TriggerTowerMap();
        arbiter_ = new SuperstripArbiter();
    }

    // Destructor
    ~PatternMatcher() {
        if (ttmap_)     delete ttmap_;
        if (arbiter_)   delete arbiter_;
    }


    // Setters
    void setNEvents(long long n)    { if (n != -1)  nEvents_     = n > 0 ? n : 0; }
    void setMinFrequency(int n)     { minFrequency_ = n > 1 ? n : 1; }
    void setMaxPatterns(int n)      { if (n != -1)  maxPatterns_ = n > 0 ? n : 0; }
    void setMaxMisses(int n)        { if (n != -1)  maxMisses_   = n > 0 ? n : 0; }
    void setMaxStubs(int n)         { if (n != -1)  maxStubs_    = n > 0 ? n : 0; }
    void setMaxRoads(int n)         { if (n != -1)  maxRoads_    = n > 0 ? n : 0; }
    void setVerbosity(int v)        { verbose_ = v; }

    // Getters
    // none

    // Main driver
    int run(TString src, TString bank, TString datadir, TString out);


  private:
    // Member functions
    // Setup trigger tower and superstrip definitions
    int setupTriggerTower(TString datadir);
    int setupSuperstrip();

    // Load pattern bank
    int loadPatterns(TString bank);

    // Do pattern recognition, write roads (patterns that fired)
    int makeRoads(TString src, TString out);

    // Configurations
    const PatternBankOption po_;
    const TString prefixRoad_;
    const TString suffix_;

    // Program options
    long long nEvents_;
    int minFrequency_;  // min frequency of patterns to be read out
    int maxPatterns_;   // max number of patterns
    int maxMisses_;     // max number of misses
    int maxStubs_;      // max number of stubs per superstrip
    int maxRoads_;      // max number of roads per event
    int verbose_;

    // Operators
    TriggerTowerMap   * ttmap_;
    SuperstripArbiter * arbiter_;

    // Pattern bank data
    std::vector<pattern_type> patternBank_;

    // Hit buffer
    std::map<unsigned, std::vector<unsigned> > hitBuffer_;
    std::vector<bool>                          hitBuffer_bool_;
};

#endif
