#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"

int copyStubs()
{
    gROOT->ProcessLine("#include <vector>");

    // Enter your file name
    TFile *tfile = new TFile("/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/PU140/Neutrino_E2023TTI_PU140_TuneCUEP6S1_ntuple.1.root");
    assert(tfile != NULL);

    // Enter your TTree name
    TTree *ttree = (TTree *) tfile->Get("ntupler/tree");
    assert(ttree != NULL);

    // Enter number of events to read
    unsigned nevents = 100;
    //unsigned nevents = ttree->GetEntries();

    // Select branches to copy
    //TObjArray* branches = ttree->GetListOfBranches();
    //for (int i=0; i<branches->GetSize(); ++i)
    //    std::cout << ((TBranch*) branches->At(i)).GetName() << std::endl;
    ttree->SetBranchStatus("*", 0);
    ttree->SetBranchStatus("TTStubs_coordx"  , 1);
    ttree->SetBranchStatus("TTStubs_coordy"  , 1);
    ttree->SetBranchStatus("TTStubs_eta"     , 1);
    ttree->SetBranchStatus("TTStubs_modId"   , 1);
    ttree->SetBranchStatus("TTStubs_phi"     , 1);
    ttree->SetBranchStatus("TTStubs_r"       , 1);
    ttree->SetBranchStatus("TTStubs_trigBend", 1);
    ttree->SetBranchStatus("TTStubs_trkId"   , 1);
    ttree->SetBranchStatus("TTStubs_z"       , 1);
    ttree->SetBranchStatus("trkParts_charge" , 1);
    ttree->SetBranchStatus("trkParts_eta"    , 1);
    ttree->SetBranchStatus("trkParts_phi"    , 1);
    ttree->SetBranchStatus("trkParts_pt"     , 1);
    ttree->SetBranchStatus("trkParts_vx"     , 1);
    ttree->SetBranchStatus("trkParts_vy"     , 1);
    ttree->SetBranchStatus("trkParts_vz"     , 1);
    ttree->SetBranchStatus("trkParts_primary", 1);

    // Enter your output file name
    TFile *newtfile = TFile::Open("nu140_100I.root", "RECREATE");
    TTree *newttree = ttree->CloneTree(0);


    // Loop over events
    for (unsigned ievt=0; ievt<nevents; ++ievt) {
        // Get a new event
        ttree->GetEntry(ievt);

        newttree->Fill();
    }

    newtfile->Write();

    return 0;
}
