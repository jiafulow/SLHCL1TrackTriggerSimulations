#ifndef NTupleTools_MapPixelDigis_h_
#define NTupleTools_MapPixelDigis_h_

#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"
#include <map>


class TrackerGeometry;

class MapPixelDigis {
  public:
    typedef edm::DetSetVector<PixelDigi> dsv_digi;
    typedef edm::DetSet<PixelDigi>       ds_digi;

    MapPixelDigis() {}
    ~MapPixelDigis() {}

    void setup(const edm::Handle<dsv_digi>& handle, const TrackerGeometry * theGeometry);

    unsigned size() const { return mapping.size(); }

    //unsigned get(const Ref_PixelDigi_ aref);
    unsigned get(const unsigned geoId, const unsigned channel);

  private:
    //std::map<Ref_PixelDigi_, unsigned> mapping;
    std::map<std::pair<unsigned, unsigned>, unsigned> mapping;
};

#endif
