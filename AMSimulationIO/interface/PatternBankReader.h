#ifndef AMSimulationIO_PatternBankReader_h_
#define AMSimulationIO_PatternBankReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"

#include "TFile.h"
#include "TROOT.h"
#include "TTree.h"
#include "TString.h"
#include <map>
#include <memory>
#include <vector>

namespace slhcl1tt {


// _____________________________________________________________________________
class PatternBankReader {
  public:
    PatternBankReader(int verbose=1);
    ~PatternBankReader();

    int init(TString src);

    void getTriggerTowerMaps(std::map<unsigned, std::vector<unsigned> >& ttmap,
                             std::map<unsigned, std::vector<unsigned> >& ttrmap);

    void getPatternBankStats(float& coverage, unsigned& count);

    Int_t getPattern(Long64_t entry) { return ttree->GetEntry(entry); }

    Long64_t getPatterns() const { return ttree->GetEntries(); }

    // Trigger tower map
    std::map<unsigned, std::vector<unsigned> > * pb_ttmap;
    std::map<unsigned, std::vector<unsigned> > * pb_ttrmap;

    // Pattern bank statistics
    float                   pb_coverage;
    unsigned                pb_count;

    // Pattern bank
    count_type              pb_frequency;
    std::vector<id_type> *  pb_superstripIds;

  private:
    TFile* tfile;
    TTree* ttree;   // for pattern bank
    TTree* ttree2;  // for pattern bank statistics
    TTree* ttree3;  // for trigger tower map
    const int verbose_;
};


// _____________________________________________________________________________
class PatternBankWriter {
  public:
    PatternBankWriter(int verbose=1);
    ~PatternBankWriter();

    int init(TString out);

    void fillTriggerTowerMaps();  // must be called before fillPatternBank()

    void fillPatternBankStats();  // must be called before fillPatternBank()

    void fillPatternBank();   // must be called after fillTriggerTower()

    Long64_t write();

    // Trigger tower map
    std::map<unsigned, std::vector<unsigned> > * pb_ttmap;
    std::map<unsigned, std::vector<unsigned> > * pb_ttrmap;

    // Pattern bank statistics
    std::auto_ptr<float>                  pb_coverage;
    std::auto_ptr<unsigned>               pb_count;

    // Pattern bank
    std::auto_ptr<count_type>             pb_frequency;
    std::auto_ptr<std::vector<id_type> >  pb_superstripIds;

  private:
    TFile* tfile;
    TTree* ttree;   // for pattern bank
    TTree* ttree2;  // for pattern bank statistics
    TTree* ttree3;  // for trigger tower map
    const int verbose_;
};

}  // namespace slhcl1tt

#endif

