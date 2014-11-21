#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"
using namespace slhcl1tt;


// _____________________________________________________________________________
TTRoadReader::TTRoadReader(int verbose)
: BasicReader(verbose),

  vr_bankIndex         (0),
  vr_triggerTowerId    (0),
  vr_nsuperstrips      (0),
  vr_stubSuperstripIds (0),
  vr_stubRefs          (0) {}

TTRoadReader::~TTRoadReader() {}

int TTRoadReader::init(TString src, TString prefix, TString suffix) {
    BasicReader::init(src);

    tchain->SetBranchAddress(prefix + "bankIndex"         + suffix, &(vr_bankIndex));
    tchain->SetBranchAddress(prefix + "triggerTowerId"    + suffix, &(vr_triggerTowerId));
    tchain->SetBranchAddress(prefix + "nsuperstrips"      + suffix, &(vr_nsuperstrips));
    tchain->SetBranchAddress(prefix + "stubSuperstripIds" + suffix, &(vr_stubSuperstripIds));
    tchain->SetBranchAddress(prefix + "stubRefs"          + suffix, &(vr_stubRefs));
    return 0;
}


// _____________________________________________________________________________
TTRoadWriter::TTRoadWriter(int verbose)
: BasicWriter(verbose),
  vr_bankIndex        (new std::vector<id_type>()),
  vr_triggerTowerId   (new std::vector<id_type>()),
  vr_nsuperstrips     (new std::vector<count_type>()),
  vr_stubSuperstripIds(new std::vector<std::vector<id_type> >()),
  vr_stubRefs         (new std::vector<std::vector<unsigned> >()) {}

TTRoadWriter::~TTRoadWriter() {}

int TTRoadWriter::init(TChain* tchain, TString out, TString prefix, TString suffix) {
    BasicWriter::init(tchain, out);

    ttree->Branch(prefix + "bankIndex"         + suffix, &(*vr_bankIndex));
    ttree->Branch(prefix + "triggerTowerId"    + suffix, &(*vr_triggerTowerId));
    ttree->Branch(prefix + "nsuperstrips"      + suffix, &(*vr_nsuperstrips));
    ttree->Branch(prefix + "stubSuperstripIds" + suffix, &(*vr_stubSuperstripIds));
    ttree->Branch(prefix + "stubRefs"          + suffix, &(*vr_stubRefs));
    return 0;
}

void TTRoadWriter::fill(const std::vector<TTRoad>& roads) {
    vr_bankIndex        ->clear();
    vr_triggerTowerId   ->clear();
    vr_nsuperstrips     ->clear();
    vr_stubSuperstripIds->clear();
    vr_stubRefs         ->clear();

    const unsigned nroads = roads.size();
    for (unsigned i=0; i<nroads; ++i) {
        const TTRoad&             road = roads.at(i);
        vr_bankIndex        ->push_back(road.bankIndex());
        vr_triggerTowerId   ->push_back(road.triggerTowerId());
        vr_nsuperstrips     ->push_back(road.nsuperstrips());
        vr_stubSuperstripIds->push_back(road.stubSuperstripIds());
        vr_stubRefs         ->push_back(road.stubRefs());
    }

    ttree->Fill();
    assert(vr_bankIndex->size() == nroads);
}
