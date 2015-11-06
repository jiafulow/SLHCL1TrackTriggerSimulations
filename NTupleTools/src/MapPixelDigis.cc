#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapPixelDigis.h"

#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"

void MapPixelDigis::setup(const edm::Handle<dsv_digi>& handle, const TrackerGeometry * theGeometry) {
    if (handle.isValid()) {
        unsigned n = 0;
        for (dsv_digi::const_iterator itv = handle->begin(); itv != handle->end(); ++itv) {
            const DetId geoId(itv->detId());
            DetId::Detector det = geoId.det();  // Tracker=1,Muon=2,Ecal=3,Hcal=4,Calo=5,Forward=6
            int subdet = geoId.subdetId();      // PXB=1,PXF=2,...
            bool isTracker = (det == DetId::Tracker);
            bool isBarrel = (subdet == (int) PixelSubdetector::PixelBarrel);
            bool isEndcap = (subdet == (int) PixelSubdetector::PixelEndcap);
            if (!isTracker)
                continue;  // only tracker
            if (!isBarrel && !isEndcap)
                continue;  // only tracker

            const PixelGeomDetUnit* geomDetUnit = dynamic_cast<const PixelGeomDetUnit*>(theGeometry->idToDetUnit(geoId));

            // This cut is as used in Geometry/TrackerGeometryBuilder/src/StackedTrackerGeometryBuilder.cc
            if (geomDetUnit->position().perp()<20.)
                continue;  // only outer tracker

            for (ds_digi::const_iterator it = itv->begin(); it != itv->end(); ++it) {
                //const Ref_PixelDigi_ aref = edm::makeRefTo(handle, geoId, it);
                //mapping.insert(std::make_pair(aref, n));

                const unsigned channel = it->channel();
                mapping.insert(std::make_pair(std::make_pair(geoId.rawId(), channel), n));

                n++;
            }
        }
    }
}

//unsigned MapPixelDigis::get(const Ref_PixelDigi_ aref) {
//    return mapping.at(aref);  // must exist
//}

unsigned MapPixelDigis::get(const unsigned geoId, const unsigned channel) {
    return mapping.at(std::make_pair(geoId, channel));  // must exist
}
