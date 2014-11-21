#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
TTStubReader::TTStubReader(int verbose)
: BasicReader(verbose) {}

TTStubReader::~TTStubReader() {
    const int n = ttformulas.GetLast()+1;
    for (int i=0; i<n; i++) {
        TTreeFormula* ttf = (TTreeFormula*) ttformulas.UncheckedAt(i);
        if (ttf)  delete ttf;
    }
}

int TTStubReader::init(TString src, bool full) {
    return BasicReader::init(src, full);
}

TTreeFormula* TTStubReader::addFormula(TString formula) {
    const int n = ttformulas.GetLast()+1;
    TTreeFormula* ttf = new TTreeFormula(Form("ttf%i", n), formula, tchain);
    //ttf->SetQuickLoad(1);
    ttformulas.Add(ttf);
    return ttf;
}

Long64_t TTStubReader::loadTree(Long64_t entry) {
    if (!tchain) return -5;
    Long64_t centry = tchain->LoadTree(entry);
    if (centry < 0) return centry;
    if (tchain->GetTreeNumber() != treenumber) {
        treenumber = tchain->GetTreeNumber();

        const int n = ttformulas.GetLast()+1;
        for (int i=0; i<n; i++) {
            TTreeFormula* ttf = (TTreeFormula*) ttformulas.UncheckedAt(i);
            ttf->UpdateFormulaLeaves();
        }
    }
    return centry;
}


// _____________________________________________________________________________
TTStubWriter::TTStubWriter(int verbose)
: BasicWriter(verbose) {}

TTStubWriter::~TTStubWriter() {}

int TTStubWriter::init(TChain* tchain, TString out) {
    return BasicWriter::init(tchain, out);
}

void TTStubWriter::fill() {
    ttree->Fill();
}
