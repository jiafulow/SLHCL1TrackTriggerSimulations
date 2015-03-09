#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
TTRoadReader::TTRoadReader(int verbose)
: BasicReader(verbose),

  vr_patternRef   (0),
  vr_tower        (0),
  vr_nstubs       (0),
  vr_superstripIds(0),
  vr_stubRefs     (0) {}

TTRoadReader::~TTRoadReader() {}

int TTRoadReader::init(TString src, TString prefix, TString suffix) {
    if (BasicReader::init(src))
        return 1;

    tchain->SetBranchAddress(prefix + "patternRef"    + suffix, &(vr_patternRef));
    tchain->SetBranchAddress(prefix + "tower"         + suffix, &(vr_tower));
    tchain->SetBranchAddress(prefix + "nstubs"        + suffix, &(vr_nstubs));
    tchain->SetBranchAddress(prefix + "superstripIds" + suffix, &(vr_superstripIds));
    tchain->SetBranchAddress(prefix + "stubRefs"      + suffix, &(vr_stubRefs));
    return 0;
}


// _____________________________________________________________________________
TTRoadWriter::TTRoadWriter(int verbose)
: BasicWriter(verbose),

  vr_patternRef     (new std::vector<unsigned>()),
  vr_tower          (new std::vector<unsigned>()),
  vr_nstubs         (new std::vector<unsigned>()),
  vr_superstripIds  (new std::vector<std::vector<unsigned> >()),
  vr_stubRefs       (new std::vector<std::vector<std::vector<unsigned> > >()) {}

TTRoadWriter::~TTRoadWriter() {}

int TTRoadWriter::init(TChain* tchain, TString out, TString prefix, TString suffix) {
    if (BasicWriter::init(tchain, out))
        return 1;

    ttree->Branch(prefix + "patternRef"    + suffix, &(*vr_patternRef));
    ttree->Branch(prefix + "tower"         + suffix, &(*vr_tower));
    ttree->Branch(prefix + "nstubs"        + suffix, &(*vr_nstubs));
    ttree->Branch(prefix + "superstripIds" + suffix, &(*vr_superstripIds));
    ttree->Branch(prefix + "stubRefs"      + suffix, &(*vr_stubRefs));
    return 0;
}

void TTRoadWriter::fill(const std::vector<TTRoad>& roads) {
    vr_patternRef   ->clear();
    vr_tower        ->clear();
    vr_nstubs       ->clear();
    vr_superstripIds->clear();
    vr_stubRefs     ->clear();

    const unsigned nroads = roads.size();
    for (unsigned i=0; i<nroads; ++i) {
        const TTRoad& road = roads.at(i);
        vr_patternRef   ->push_back(road.patternRef);
        vr_tower        ->push_back(road.tower);
        vr_nstubs       ->push_back(road.nstubs);
        vr_superstripIds->push_back(road.superstripIds);
        vr_stubRefs     ->push_back(road.stubRefs);
    }

    ttree->Fill();
    assert(vr_patternRef->size() == nroads);
}
