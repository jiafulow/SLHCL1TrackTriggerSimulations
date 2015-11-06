#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTTClusters.h"

void MapTTClusters::setup(const edm::Handle<dsv_clus>& handle) {
    if (handle.isValid()) {
        unsigned n = 0;
        for (dsv_clus::const_iterator itv = handle->begin(); itv != handle->end(); ++itv) {
            for (ds_clus::const_iterator it = itv->begin(); it != itv->end(); ++it) {
                //const ref_clus aref = edmNew::makeRefTo(handle, it);
                //mapping.insert(std::make_pair(aref, n));

                const ref_clus aref = edmNew::makeRefTo(handle, it);
                mapping.insert(std::make_pair(std::make_pair(aref->getDetId(), aref->getHits()), n));

                n++;
            }
        }
    }
}

//unsigned MapTTClusters::get(const ref_clus aref) {
//    return mapping.at(aref);  // must exist
//}

unsigned MapTTClusters::get(const ref_clus aref) {
    return mapping.at(std::make_pair(aref->getDetId(), aref->getHits()));
}

