#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
TTTrackReader::TTTrackReader(int verbose)
: TTRoadReader(verbose) {}

TTTrackReader::~TTTrackReader() {}

int TTTrackReader::init(TString src, TString prefixRoad, TString prefixTrack, TString suffix) {
    if (TTRoadReader::init(src, prefixRoad, suffix))
        return 1;

    // FIXME
    return 0;
}


// _____________________________________________________________________________
TTTrackWriter::TTTrackWriter(int verbose)
: BasicWriter(verbose),

  vt_px           (new std::vector<float>()),
  vt_py           (new std::vector<float>()),
  vt_pz           (new std::vector<float>()),
  vt_pt           (new std::vector<float>()),
  vt_eta          (new std::vector<float>()),
  vt_phi          (new std::vector<float>()),
  vt_vx           (new std::vector<float>()),
  vt_vy           (new std::vector<float>()),
  vt_vz           (new std::vector<float>()),
  vt_rinv         (new std::vector<float>()),
  vt_phi0         (new std::vector<float>()),
  vt_cottheta     (new std::vector<float>()),
  vt_z0           (new std::vector<float>()),
  vt_d0           (new std::vector<float>()),
  vt_chi2         (new std::vector<float>()),
  vt_ndof         (new std::vector<int>()),
  vt_chi2_phi     (new std::vector<float>()),
  vt_chi2_z       (new std::vector<float>()),
  vt_tpId         (new std::vector<int>()),
  vt_tower        (new std::vector<unsigned>()),
  vt_roadRef      (new std::vector<unsigned>()),
  vt_stubRefs     (new std::vector<std::vector<unsigned> >()) {}


TTTrackWriter::~TTTrackWriter() {}

int TTTrackWriter::init(TChain* tchain, TString out, TString prefix, TString suffix) {
    if (BasicWriter::init(tchain, out))
        return 1;

  //ttree->Branch(prefix + "px"             + suffix, &(*vt_px));
  //ttree->Branch(prefix + "py"             + suffix, &(*vt_py));
  //ttree->Branch(prefix + "pz"             + suffix, &(*vt_pz));
    ttree->Branch(prefix + "pt"             + suffix, &(*vt_pt));
    ttree->Branch(prefix + "eta"            + suffix, &(*vt_eta));
  //ttree->Branch(prefix + "phi"            + suffix, &(*vt_phi));
  //ttree->Branch(prefix + "vx"             + suffix, &(*vt_vx));
  //ttree->Branch(prefix + "vy"             + suffix, &(*vt_vy));
  //ttree->Branch(prefix + "vz"             + suffix, &(*vt_vz));
    ttree->Branch(prefix + "rinv"           + suffix, &(*vt_rinv));
    ttree->Branch(prefix + "phi0"           + suffix, &(*vt_phi0));
    ttree->Branch(prefix + "cottheta"       + suffix, &(*vt_cottheta));
    ttree->Branch(prefix + "z0"             + suffix, &(*vt_z0));
    ttree->Branch(prefix + "d0"             + suffix, &(*vt_d0));
    ttree->Branch(prefix + "chi2"           + suffix, &(*vt_chi2));
    ttree->Branch(prefix + "ndof"           + suffix, &(*vt_ndof));
    ttree->Branch(prefix + "chi2_phi"       + suffix, &(*vt_chi2_phi));
    ttree->Branch(prefix + "chi2_z"         + suffix, &(*vt_chi2_z));
    ttree->Branch(prefix + "tpId"           + suffix, &(*vt_tpId));
    ttree->Branch(prefix + "tower"          + suffix, &(*vt_tower));
    ttree->Branch(prefix + "roadRef"        + suffix, &(*vt_roadRef));
    ttree->Branch(prefix + "stubRefs"       + suffix, &(*vt_stubRefs));
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
    vt_phi0            ->clear();
    vt_cottheta        ->clear();
    vt_z0              ->clear();
    vt_d0              ->clear();
    vt_chi2            ->clear();
    vt_ndof            ->clear();
    vt_chi2_phi        ->clear();
    vt_chi2_z          ->clear();
    vt_tpId            ->clear();
    vt_tower           ->clear();
    vt_roadRef         ->clear();
    vt_stubRefs        ->clear();

    const unsigned ntracks = tracks.size();
    for (unsigned i=0; i<ntracks; ++i) {
        const TTTrack& track = tracks.at(i);
        const GlobalVector& momentum = track.getMomentum();
        const GlobalPoint&  poca = track.getPOCA();
        const TTTrackParam& param = track.getTrackParam();

        //vt_px              ->push_back(momentum.x());
        //vt_py              ->push_back(momentum.y());
        //vt_pz              ->push_back(momentum.z());
        vt_pt              ->push_back(std::sqrt(momentum.perp2()));
        vt_eta             ->push_back(momentum.eta());
        //vt_phi             ->push_back(momentum.phi());
        //vt_vx              ->push_back(poca.x());
        //vt_vy              ->push_back(poca.y());
        //vt_vz              ->push_back(poca.z());
        vt_rinv            ->push_back(param.rinv);
        vt_phi0            ->push_back(param.phi0);
        vt_cottheta        ->push_back(param.cottheta);
        vt_z0              ->push_back(param.z0);
        vt_d0              ->push_back(param.d0);
        vt_chi2            ->push_back(param.chi2);
        vt_ndof            ->push_back(param.ndof);
        vt_chi2_phi        ->push_back(-999999.);  // dummy
        vt_chi2_z          ->push_back(-999999.);  // dummy
        vt_tpId            ->push_back(-1);        // dummy
        //vt_tower           ->push_back(track.getSector());
        vt_roadRef         ->push_back(track.getRoadRef());
        vt_stubRefs        ->push_back(track.getStubRefs());
    }

    ttree->Fill();
    assert(vt_px->size() == ntracks);
}

void TTTrackWriter::fill(const std::vector<TTTrack2>& tracks) {
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
    vt_phi0            ->clear();
    vt_cottheta        ->clear();
    vt_z0              ->clear();
    vt_d0              ->clear();
    vt_chi2            ->clear();
    vt_ndof            ->clear();
    vt_chi2_phi        ->clear();
    vt_chi2_z          ->clear();
    vt_tpId            ->clear();
    vt_tower           ->clear();
    vt_roadRef         ->clear();
    vt_stubRefs        ->clear();

    const unsigned ntracks = tracks.size();
    for (unsigned i=0; i<ntracks; ++i) {
        const TTTrack2& track = tracks.at(i);

        //vt_px              ->push_back(track.px());
        //vt_py              ->push_back(track.py());
        //vt_pz              ->push_back(track.pz());
        vt_pt              ->push_back(track.pt());
        vt_eta             ->push_back(track.eta());
        //vt_phi             ->push_back(track.phi());
        //vt_vx              ->push_back(track.vx());
        //vt_vy              ->push_back(track.vy());
        //vt_vz              ->push_back(track.vz());
        vt_rinv            ->push_back(track.rinv());
        vt_phi0            ->push_back(track.phi0());
        vt_cottheta        ->push_back(track.cottheta());
        vt_z0              ->push_back(track.z0());
        vt_d0              ->push_back(track.d0());
        vt_chi2            ->push_back(track.chi2());
        vt_ndof            ->push_back(track.ndof());
        vt_chi2_phi        ->push_back(track.chi2_phi());
        vt_chi2_z          ->push_back(track.chi2_z());
        vt_tpId            ->push_back(track.tpId());
        vt_tower           ->push_back(track.tower());
        vt_roadRef         ->push_back(track.roadRef());
        vt_stubRefs        ->push_back(track.stubRefs());
    }

    ttree->Fill();
    assert(vt_pt->size() == ntracks);
}
