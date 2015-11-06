#ifndef NTupleTools_MapMixedSimHits_h_
#define NTupleTools_MapMixedSimHits_h_

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include <map>


class TrackerGeometry;

class MapMixedSimHits {
  public:
    MapMixedSimHits() {}
    ~MapMixedSimHits() {}

    void setup(const std::vector<edm::InputTag>& simHitCollections, const edm::Event& iEvent);

    unsigned size() const { return mapping.size(); }

    unsigned get(const unsigned subdet, const unsigned tofBin, const unsigned CFposition);

  private:
    std::map<unsigned, unsigned> mapping;
};

#endif
