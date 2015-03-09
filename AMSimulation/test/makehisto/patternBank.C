#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TH1D.h"

void patternBank() {
    //TString infilename = "patternBank_200x1_ttt27_198M.2GeV.root";   double finalCoverage = 0.99923;
    //TString infilename = "patternBank_400x1_ttt27_198M.2GeV.root";   double finalCoverage = 0.99108;
    //TString infilename = "patternBank_200x1_ttt27_198M.10GeV.root";  double finalCoverage = 0.99944;
    //TString infilename = "patternBank_400x1_ttt27_198M.10GeV.root";  double finalCoverage = 0.99841;

    //TString infilename = "patternBank_200x1_ttt27zm_38M.2GeV.root";  double finalCoverage = 0.99715;
    //TString infilename = "patternBank_400x1_ttt27zm_38M.2GeV.root";  double finalCoverage = 0.96221;
    //TString infilename = "patternBank_200x1_ttt27zm_38M.10GeV.root"; double finalCoverage = 0.99954;
    //TString infilename = "patternBank_400x1_ttt27zm_38M.10GeV.root"; double finalCoverage = 0.99377;

    //TString infilename = "patternBank_sp16_ss512_tt27_250M.root";   double finalCoverage = 0.99975;
    TString infilename = "patternBank_sp16_ss512_tt27_16M.root";   double finalCoverage = 0.99489;

    TString outfilename = infilename; outfilename.ReplaceAll("patternBank_", "histos_");
    TFile *infile = TFile::Open(infilename);
    TTree *ttree = (TTree *) infile->Get("patternBank");

    unsigned short frequency;
    TBranch *b_frequency;
    TH1D *h_coverage = new TH1D("h_coverage", "Coverage", 1e4, 0, 1e7);  // step size = 1e3

    ttree->SetBranchAddress("frequency", &frequency, &b_frequency);
    h_coverage->SetDirectory(0);
    const long long nentries = ttree->GetEntries();
    std::cout << "num of patterns: " << nentries << std::endl;

    unsigned int sum = 0;
    for (long long ievt=0; ievt<nentries; ++ievt) {
        b_frequency->GetEntry(ievt);

        sum += frequency;
    }
    std::cout << "integral: " << sum << std::endl;

    unsigned int sum2 = 0;
    double coverage;
    for (long long ievt=0; ievt<nentries; ++ievt) {
        if (ievt % 1000 == 0) {
            coverage = sum2;
            coverage /= sum;
            h_coverage->SetBinContent(ievt/1000 + 1, coverage * finalCoverage);
        }

        b_frequency->GetEntry(ievt);
        sum2 += frequency;
    }
    std::cout << "final coverage: " << coverage << std::endl;
    infile->Close();

    TFile *outfile = TFile::Open(outfilename, "RECREATE");
    h_coverage->Write();
    outfile->Close();
}
