#ifndef AMSimulation_StubCleaner_h_
#define AMSimulation_StubCleaner_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/HelperMath.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternBankOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Picky.h"
using namespace slhcl1tt;


class StubCleaner {
  public:
    // Constructor
    StubCleaner(PatternBankOption po)
    : po_(po),
      nEvents_(999999999), verbose_(1) {

        // Set event selection (default: always pass)
        eventSelect_ = "(1)";

        // Initialize
        picky_ = new Picky();
    }

    // Destructor
    ~StubCleaner() {
        if (picky_)  delete picky_;
    }


    // Setters
    void setNEvents(long long n)    { if (n != -1)  nEvents_ = n > 0 ? n : 0; }
    void setVerbosity(int v)        { verbose_ = v; }

    // Getters
    // none

    // Main driver
    int run(TString src, TString out);


  private:
    // Member functions
    // Select one unique stub per layer
    int cleanStubs(TString src, TString out);

    // Configurations
    const PatternBankOption po_;

    // Program options
    long long nEvents_;
    int verbose_;

    // Event selection
    TString eventSelect_;

    // Picky
    Picky * picky_;
};

#endif
