#ifndef AMSimulation_Converter_h_
#define AMSimulation_Converter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
using namespace slhcl1tt;

#include "TFile.h"

class Converter {
  public:
    // Constructor, destructor
    Converter():
      bankName_("patternBank"),
      nEvents_(999999999), skipEvents_(0),
      verbose_(1) {}

    ~Converter() {}


    // Setters
    void setNEvents(long long n)    { if (n != -1)  nEvents_    = n > 0 ? n : 0; }
    void setSkipEvents(long long n) { if (n != -1)  skipEvents_ = n > 0 ? n : 0; }
    void setVerbosity(int v)        { verbose_ = v; }

    // Getters
    // none

    // Functions
    int bankToTxt(TString src, TString out, TString fmt);


  private:
    const TString bankName_;

    // Program options
    long long nEvents_;
    long long skipEvents_;
    int verbose_;
};

#endif // AMSimulation_Converter_h_
