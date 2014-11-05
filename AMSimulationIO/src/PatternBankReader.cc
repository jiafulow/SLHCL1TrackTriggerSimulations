#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
PatternBankReader::PatternBankReader(int verbose)
: pb_ttmap          (0),
  pb_ttrmap         (0),
  //
  pb_coverage       (0.),
  pb_count          (0),
  //
  pb_frequency      (0),
  pb_superstripIds  (0),
  //
  verbose_(verbose) {}

PatternBankReader::~PatternBankReader() {
    if (ttree3) delete ttree3;
    if (ttree2) delete ttree2;
    if (ttree)  delete ttree;
    if (tfile)  delete tfile;
}

int PatternBankReader::init(TString src) {
    if (!src.EndsWith(".root")) {
        std::cout << Error() << "Input source must be .root" << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << "Opening " << src << std::endl;
    tfile = TFile::Open(src);

    if (tfile) {
        if (verbose_)  std::cout << Info() << "Successfully opened " << src << std::endl;
    } else {
        std::cout << Error() << "Failed to open " << src << std::endl;
        return 1;
    }

    ttree3 = (TTree*) tfile->Get("triggerTower");
    assert(ttree3 != 0);

    ttree3->SetBranchAddress("triggerTowerMap"       , &pb_ttmap);
    ttree3->SetBranchAddress("triggerTowerReverseMap", &pb_ttrmap);

    ttree2 = (TTree*) tfile->Get("patternBankStats");
    assert(ttree2 != 0);

    ttree2->SetBranchAddress("coverage", &pb_coverage);
    ttree2->SetBranchAddress("count"   , &pb_count);

    ttree = (TTree*) tfile->Get("patternBank");
    assert(ttree != 0);

    ttree->SetBranchAddress("frequency"    , &pb_frequency);
    ttree->SetBranchAddress("superstripIds", &pb_superstripIds);

    return 0;
}

void PatternBankReader::getTriggerTowerMaps(std::map<unsigned, std::vector<unsigned> >& ttmap,
                                            std::map<unsigned, std::vector<unsigned> >& ttrmap) {
    ttree3->GetEntry(0);

    ttmap.swap(*pb_ttmap);
    ttrmap.swap(*pb_ttrmap);
}

void PatternBankReader::getPatternBankStats(float& coverage, unsigned& count) {
    ttree2->GetEntry(0);

    coverage = pb_coverage;
    count = pb_count;
}


// _____________________________________________________________________________
PatternBankWriter::PatternBankWriter(int verbose)
: pb_ttmap          (0),
  pb_ttrmap         (0),
  //
  pb_coverage       (new float(0.)),
  pb_count          (new unsigned(0)),
  //
  pb_frequency      (new count_type(0)),
  pb_superstripIds  (new std::vector<id_type>()),
  //
  verbose_(verbose) {}

PatternBankWriter::~PatternBankWriter() {
    if (ttree3) delete ttree3;
    if (ttree2) delete ttree2;
    if (ttree)  delete ttree;
    if (tfile)  delete tfile;
}

int PatternBankWriter::init(TString out) {
    gROOT->ProcessLine("#include <vector>");  // how is it not loaded?

    if (!out.EndsWith(".root")) {
        std::cout << Error() << "Output filename must be .root" << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << "Opening " << out << std::endl;
    tfile = TFile::Open(out, "RECREATE");

    if (tfile) {
        if (verbose_)  std::cout << Info() << "Successfully recreated " << out << std::endl;
    } else {
        std::cout << Error() << "Failed to recreate " << out << std::endl;
        return 1;
    }

    // Trigger tower map
    ttree3 = new TTree("triggerTower", "");

    // Pattern bank statistics
    ttree2 = new TTree("patternBankStats", "");

    // Pattern bank
    ttree = new TTree("patternBank", "");
    ttree->Branch("frequency"              , &(*pb_frequency));
    ttree->Branch("superstripIds"          , &(*pb_superstripIds));

    return 0;
}

void PatternBankWriter::fillTriggerTowerMaps() {
    ttree3->Branch("triggerTowerMap"       , pb_ttmap);
    ttree3->Branch("triggerTowerReverseMap", pb_ttrmap);
    ttree3->Fill();
    assert(ttree3->GetEntries() == 1);
}

void PatternBankWriter::fillPatternBankStats() {
    ttree2->Branch("coverage", &(*pb_coverage));
    ttree2->Branch("count"   , &(*pb_count));
    ttree2->Fill();
    assert(ttree2->GetEntries() == 1);
}

void PatternBankWriter::fillPatternBank() {
    ttree->Fill();
}

Long64_t PatternBankWriter::write() {
    Long64_t nentries = ttree->GetEntries();
    tfile->Write();
    //tfile->Close();
    return nentries;
}

