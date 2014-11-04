#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
TTTrackReader::TTTrackReader(int verbose)
:
  verbose_(verbose) {}

TTTrackReader::~TTTrackReader() {
    if (tchain)  delete tchain;
}

// _____________________________________________________________________________
TTTrackWriter::TTTrackWriter(int verbose)
: vt_px           (new std::vector<float>()),
  vt_py           (new std::vector<float>()),
  vt_pz           (new std::vector<float>()),
  vt_pt           (new std::vector<float>()),
  vt_eta          (new std::vector<float>()),
  vt_phi          (new std::vector<float>()),
  vt_vx           (new std::vector<float>()),
  vt_vy           (new std::vector<float>()),
  vt_vz           (new std::vector<float>()),
  vt_rinv         (new std::vector<float>()),
  vt_chi2         (new std::vector<float>()),
  vt_ptconsistency(new std::vector<float>()),
  vt_nstubs       (new std::vector<unsigned>()),
  verbose_(verbose) {}

TTTrackWriter::~TTTrackWriter() {
    if (ttree)  delete ttree;
    if (tfile)  delete tfile;
}

int TTTrackWriter::init(TString out, TString prefix, TString suffix) {
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
    ttree = new TTree("tree", "");

    ttree->Branch(prefix + "px"             + suffix, &(*vt_px));
    ttree->Branch(prefix + "py"             + suffix, &(*vt_py));
    ttree->Branch(prefix + "pz"             + suffix, &(*vt_pz));
    ttree->Branch(prefix + "pt"             + suffix, &(*vt_pt));
    ttree->Branch(prefix + "eta"            + suffix, &(*vt_eta));
    ttree->Branch(prefix + "phi"            + suffix, &(*vt_phi));
    ttree->Branch(prefix + "vx"             + suffix, &(*vt_vx));
    ttree->Branch(prefix + "vy"             + suffix, &(*vt_vy));
    ttree->Branch(prefix + "vz"             + suffix, &(*vt_vz));
    ttree->Branch(prefix + "rinv"           + suffix, &(*vt_rinv));
    ttree->Branch(prefix + "chi2"           + suffix, &(*vt_chi2));
    ttree->Branch(prefix + "ptconsistency"  + suffix, &(*vt_ptconsistency));
    ttree->Branch(prefix + "nstubs"         + suffix, &(*vt_nstubs));
    return 0;
}

void TTTrackWriter::fill(const std::vector<TTTrack>& tracks) {
    vt_px              ->clear();
    vt_py              ->clear();
    vt_pz              ->clear();
    vt_pt              ->clear();
    vt_eta             ->clear();
    vt_phi             ->clear();
    vt_vx              ->clear();
    vt_vy              ->clear();
    vt_vz              ->clear();
    vt_rinv            ->clear();
    vt_chi2            ->clear();
    vt_ptconsistency   ->clear();
    vt_nstubs          ->clear();

    const unsigned ntracks = tracks.size();
    for (unsigned i=0; i<ntracks; ++i) {
        const TTTrack& track = tracks.at(i);
        const GlobalVector& momentum = track.getMomentum();
        const GlobalPoint&  poca = track.getPOCA();

        vt_px              ->push_back(momentum.x());
        vt_py              ->push_back(momentum.y());
        vt_pz              ->push_back(momentum.z());
        vt_pt              ->push_back(std::sqrt(momentum.perp2()));
        vt_eta             ->push_back(momentum.eta());
        vt_phi             ->push_back(momentum.phi());
        vt_vx              ->push_back(poca.x());
        vt_vy              ->push_back(poca.y());
        vt_vz              ->push_back(poca.z());
        vt_rinv            ->push_back(track.getRInv());
        vt_chi2            ->push_back(track.getChi2());
        vt_ptconsistency   ->push_back(track.getStubPtConsistency());
        vt_nstubs          ->push_back(track.getStubRefs().size());
    }

    ttree->Fill();
    assert(vt_px->size() == ntracks);
}

long long TTTrackWriter::write() {
    Long64_t nentries = ttree->GetEntries();
    tfile->Write();
    //tfile->Close();
    return nentries;
}
