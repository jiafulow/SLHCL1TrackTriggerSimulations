#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/BasicReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
BasicReader::BasicReader(int verbose)
: vp_pt               (0),
  vp_eta              (0),
  vp_phi              (0),
  vp_vx               (0),
  vp_vy               (0),
  vp_vz               (0),
  vp_charge           (0),
  //
  vb_x                (0),
  vb_y                (0),
  vb_z                (0),
  vb_r                (0),
  vb_eta              (0),
  vb_phi              (0),
  vb_coordx           (0),
  vb_coordy           (0),
  vb_roughPt          (0),
  vb_trigBend         (0),
  vb_clusWidth        (0),
  vb_modId            (0),
  vb_tpId             (0),
  //
  verbose_(verbose) {}

BasicReader::~BasicReader() {
    if (tchain)  delete tchain;
}

int BasicReader::init(TString src, bool full) {
    if (!src.EndsWith(".root") && !src.EndsWith(".txt")) {
        std::cout << Error() << "Input source must be either .root or .txt" << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << "Opening " << src << std::endl;
    tchain = new TChain("ntupler/tree");

    if (src.EndsWith(".root")) {
        if (tchain->Add(src) ) {
            if (verbose_)  std::cout << Info() << "Successfully read " << src << std::endl;
        } else {
            std::cout << Error() << "Failed to read " << src << std::endl;
            return 1;
        }

    } else if (src.EndsWith(".txt")) {
        TFileCollection fc("fileinfolist", "", src);
        if (tchain->AddFileInfoList((TCollection*) fc.GetList()) ) {
            if (verbose_)  std::cout << Info() << "Successfully read " << src << std::endl;
        } else {
            std::cout << Error() << "Failed to read " << src << std::endl;
            return 1;
        }
    }

    assert(tchain != 0);
    treenumber = tchain->GetTreeNumber();

    tchain->SetBranchAddress("genParts_pt"      , &(vp_pt));
    tchain->SetBranchAddress("genParts_eta"     , &(vp_eta));
    tchain->SetBranchAddress("genParts_phi"     , &(vp_phi));
    tchain->SetBranchAddress("genParts_vx"      , &(vp_vx));
    tchain->SetBranchAddress("genParts_vy"      , &(vp_vy));
    tchain->SetBranchAddress("genParts_vz"      , &(vp_vz));
    tchain->SetBranchAddress("genParts_charge"  , &(vp_charge));
    if (full)  tchain->SetBranchAddress("TTStubs_x"        , &(vb_x));
    if (full)  tchain->SetBranchAddress("TTStubs_y"        , &(vb_y));
    tchain->SetBranchAddress("TTStubs_z"        , &(vb_z));
    tchain->SetBranchAddress("TTStubs_r"        , &(vb_r));
    tchain->SetBranchAddress("TTStubs_eta"      , &(vb_eta));
    tchain->SetBranchAddress("TTStubs_phi"      , &(vb_phi));
    tchain->SetBranchAddress("TTStubs_coordx"   , &(vb_coordx));
    tchain->SetBranchAddress("TTStubs_coordy"   , &(vb_coordy));
    if (full)  tchain->SetBranchAddress("TTStubs_roughPt"  , &(vb_roughPt));
    tchain->SetBranchAddress("TTStubs_trigBend" , &(vb_trigBend));
  //if (full)  tchain->SetBranchAddress("TTStubs_clusWidth", &(vb_clusWidth));
    tchain->SetBranchAddress("TTStubs_modId"    , &(vb_modId));
    tchain->SetBranchAddress("TTStubs_tpId"     , &(vb_tpId));
    return 0;
}


// _____________________________________________________________________________
BasicWriter::BasicWriter(int verbose)
: verbose_(verbose) {}

BasicWriter::~BasicWriter() {
    if (ttree)  delete ttree;
    if (tfile)  delete tfile;
}

int BasicWriter::init(TChain* tchain, TString out) {
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

    tfile->mkdir("ntupler")->cd();
    ttree = (TTree*) tchain->CloneTree(0); // Do not copy the data yet
    // The clone should not delete any shared i/o buffers.
    ResetDeleteBranches(ttree);
    return 0;
}

Long64_t BasicWriter::writeTree() {
    Long64_t nentries = ttree->GetEntries();
    tfile->Write();
    //tfile->Close();
    return nentries;
}
