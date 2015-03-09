#ifndef AMSimulation_PatternGenerator_h_
#define AMSimulation_PatternGenerator_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Pattern.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TriggerTowerMap.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/SuperstripArbiter.h"
using namespace slhcl1tt;


class PatternGenerator {
  public:
    // Constructor
    PatternGenerator(PatternBankOption po)
    : po_(po),
      nEvents_(999999999), minFrequency_(1), verbose_(1) {

        // Initialize
        ttmap_   = new TriggerTowerMap();
        arbiter_ = new SuperstripArbiter();
    }

    // Destructor
    ~PatternGenerator() {
        if (ttmap_)     delete ttmap_;
        if (arbiter_)   delete arbiter_;
    }


    // Setters
    void setNEvents(long long n)    { if (n != -1)  nEvents_ = n > 0 ? n : 0; }
    void setMinFrequency(int n)     { minFrequency_ = n > 1 ? n : 1; }
    void setVerbosity(int v)        { verbose_ = v; }

    // Getters
    // none

    // Main driver
    int run(TString src, TString datadir, TString out);


  private:
    // Member functions
    // Setup trigger tower and superstrip definitions
    int setupTriggerTower(TString datadir);
    int setupSuperstrip();

    // Generate pattern bank
    int makePatterns(TString src);

    // Write pattern bank
    int writePatterns(TString out);

    // Configurations
    const PatternBankOption po_;

    // Program options
    long long nEvents_;
    int minFrequency_;  // min frequency of a pattern to be stored
    int verbose_;

    // Operators
    TriggerTowerMap   * ttmap_;
    SuperstripArbiter * arbiter_;

    // Pattern bank data
    std::map<pattern_type, unsigned>                patternBank_map_;
    std::vector<std::pair<pattern_type, unsigned> > patternBank_pairs_;

    // Bookkeepers
    float coverage_;
    unsigned coverage_count_;
};

#endif
