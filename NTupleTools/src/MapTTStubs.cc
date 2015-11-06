#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTTStubs.h"

void MapTTStubs::setup(const edm::Handle<dsv_stub>& handle) {
    if (handle.isValid()) {
        unsigned n = 0;
        for (dsv_stub::const_iterator itv = handle->begin(); itv != handle->end(); ++itv) {
            for (ds_stub::const_iterator it = itv->begin(); it != itv->end(); ++it) {
                const ref_stub aref = edmNew::makeRefTo(handle, it);
                mapping.insert(std::make_pair(aref, n));

                n++;
            }
        }
    }
}

unsigned MapTTStubs::get(const ref_stub aref) {
    return mapping.at(aref);  // must exist
}
