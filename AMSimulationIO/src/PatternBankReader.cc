#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
PatternBankReader::PatternBankReader(int verbose)
: pb_coverage       (0.),
  pb_count          (0),
  pb_tower          (0),
  pb_superstrip     (0),
  //
  pb_frequency      (0),
  pb_superstripIds  (0),
  //
  verbose_(verbose) {}

PatternBankReader::~PatternBankReader() {
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

    ttree2 = (TTree*) tfile->Get("patternBankInfo");
    assert(ttree2 != 0);

    ttree2->SetBranchAddress("coverage"  , &pb_coverage);
    ttree2->SetBranchAddress("count"     , &pb_count);
    ttree2->SetBranchAddress("tower"     , &pb_tower);
    ttree2->SetBranchAddress("superstrip", &pb_superstrip);

    ttree = (TTree*) tfile->Get("patternBank");
    assert(ttree != 0);

    ttree->SetBranchAddress("frequency"    , &pb_frequency);
    ttree->SetBranchAddress("superstripIds", &pb_superstripIds);

    return 0;
}

void PatternBankReader::getPatternBankInfo(float& coverage, unsigned& count, unsigned& tower, std::string& superstrip) {
    ttree2->GetEntry(0);

    coverage   = pb_coverage;
    count      = pb_count;
    tower      = pb_tower;
    superstrip = (*pb_superstrip);
}


// _____________________________________________________________________________
PatternBankWriter::PatternBankWriter(int verbose)
: pb_coverage       (new float(0.)),
  pb_count          (new unsigned(0)),
  pb_tower          (new unsigned(0)),
  pb_superstrip     (new std::string("")),
  //
  pb_frequency      (new frequency_type(0)),
  pb_superstripIds  (new std::vector<superstrip_type>()),
  //
  verbose_(verbose) {}

PatternBankWriter::~PatternBankWriter() {
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
        if (verbose_)  std::cout << Info() << "Successfully opened " << out << std::endl;
    } else {
        std::cout << Error() << "Failed to open " << out << std::endl;
        return 1;
    }

    // Pattern bank statistics
    ttree2 = new TTree("patternBankInfo", "");

    // Pattern bank
    ttree = new TTree("patternBank", "");
    ttree->Branch("frequency"              , &(*pb_frequency));
    ttree->Branch("superstripIds"          , &(*pb_superstripIds));

    return 0;
}

void PatternBankWriter::fillPatternBankInfo() {
    ttree2->Branch("coverage"  , &(*pb_coverage));
    ttree2->Branch("count"     , &(*pb_count));
    ttree2->Branch("tower"     , &(*pb_tower));
    ttree2->Branch("superstrip", &(*pb_superstrip));
    ttree2->Fill();
    assert(ttree2->GetEntries() == 1);
}

void PatternBankWriter::fillPatternBank() {
    ttree->Fill();
}

Long64_t PatternBankWriter::writeTree() {
    Long64_t nentries = ttree->GetEntries();
    tfile->Write();
    //tfile->Close();
    return nentries;
}
