#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapMixedSimHits.h"

#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "SimDataFormats/CrossingFrame/interface/CrossingFrame.h"
#include "SimDataFormats/CrossingFrame/interface/MixCollection.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"

void MapMixedSimHits::setup(const std::vector<edm::InputTag>& simHitCollections, const edm::Event& iEvent) {
    unsigned n = 0;
    for (unsigned ict=0; ict<simHitCollections.size(); ++ict) {
        const edm::InputTag collectionTag = simHitCollections.at(ict);
        edm::Handle<CrossingFrame<PSimHit> > simHits;
        iEvent.getByLabel(collectionTag, simHits);

        if (simHits.isValid()) {
            std::auto_ptr<MixCollection<PSimHit> > mixedSimHits(new MixCollection<PSimHit>(simHits.product()));

            unsigned CFposition = 0;
            for (MixCollection<PSimHit>::iterator it = mixedSimHits->begin(); it != mixedSimHits->end(); ++it, ++CFposition) {
                const DetId geoId(it->detUnitId());
                DetId::Detector det = geoId.det();  // Tracker=1,Muon=2,Ecal=3,Hcal=4,Calo=5,Forward=6
                int subdet = geoId.subdetId();      // PXB=1,PXF=2,...
                bool isTracker = (det == DetId::Tracker);
                bool isBarrel = (subdet == (int) PixelSubdetector::PixelBarrel);
                bool isEndcap = (subdet == (int) PixelSubdetector::PixelEndcap);
                bool tofBin = (collectionTag.instance().find(std::string("HighTof")) != std::string::npos);
                if (!isTracker)
                    continue;  // only tracker
                if (!isBarrel && !isEndcap)
                    continue;  // only tracker

                const unsigned anId = (unsigned(subdet) << 24) + (unsigned(tofBin) << 23) + CFposition;
                mapping.insert(std::make_pair(anId, n));

                n++;
            }
        }
    }
}

unsigned MapMixedSimHits::get(const unsigned subdet, const unsigned tofBin, const unsigned CFposition) {
    const unsigned anId = (unsigned(subdet) << 24) + (unsigned(tofBin) << 23) + CFposition;
    return mapping.at(anId);  // must exist
}
