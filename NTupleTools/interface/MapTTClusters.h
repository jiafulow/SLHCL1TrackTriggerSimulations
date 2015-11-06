#ifndef NTupleTools_MapTTClusters_h_
#define NTupleTools_MapTTClusters_h_

#include "DataFormats/L1TrackTrigger/interface/TTCluster.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include <map>


class MapTTClusters {
  public:
    typedef edmNew::DetSetVector<TTCluster<Ref_PixelDigi_> > dsv_clus;
    typedef edmNew::DetSet<TTCluster<Ref_PixelDigi_> >       ds_clus;
    typedef edm::Ref<dsv_clus, TTCluster<Ref_PixelDigi_> >   ref_clus;

    MapTTClusters() {}
    ~MapTTClusters() {}

    void setup(const edm::Handle<dsv_clus>& handle);

    unsigned size() const { return mapping.size(); }

    unsigned get(const ref_clus aref);

  private:
    //std::map<ref_clus, unsigned> mapping;
    std::map<std::pair<DetId, std::vector<Ref_PixelDigi_> >, unsigned> mapping;
};

#endif
