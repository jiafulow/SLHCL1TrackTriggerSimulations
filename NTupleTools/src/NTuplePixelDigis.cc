#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTuplePixelDigis.h"

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/ModuleIdFunctor.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTrackingParticles.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapMixedSimHits.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"
#include "SimDataFormats/TrackerDigiSimLink/interface/PixelDigiSimLink.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"


// _____________________________________________________________________________
NTuplePixelDigis::NTuplePixelDigis(const edm::ParameterSet& iConfig) :
  inputTag_  (iConfig.getParameter<edm::InputTag>("inputTag")),
  inputTagTP_(iConfig.getParameter<edm::InputTag>("inputTagTP")),
  simHitCollectionConfig_(iConfig.getParameter<edm::ParameterSet>("simHitCollections")),
  prefix_    (iConfig.getParameter<std::string>("prefix")),
  suffix_    (iConfig.getParameter<std::string>("suffix")),
  selector_  (iConfig.existsAs<std::string>("cut") ? iConfig.getParameter<std::string>("cut") : "", true),
  maxN_      (iConfig.getParameter<unsigned>("maxN")) {

    const std::vector<std::string>& parameterNames = simHitCollectionConfig_.getParameterNames();
    for (unsigned ipn=0; ipn<parameterNames.size(); ++ipn) {
        const std::string parameterName = parameterNames.at(ipn);
        const std::vector<edm::InputTag>& collectionTags = simHitCollectionConfig_.getParameter<std::vector<edm::InputTag> >(parameterName);
        for (unsigned ict=0; ict<collectionTags.size(); ++ict) {
            const edm::InputTag collectionTag = collectionTags.at(ict);
            simHitCollections_.push_back(collectionTag);
        }
    }

    produces<std::vector<float> >                   (prefix_ + "x"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "y"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "z"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "r"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "eta"            + suffix_);
    produces<std::vector<float> >                   (prefix_ + "phi"            + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "localx"         + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "localy"         + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "localz"         + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "modId"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "geoId"          + suffix_);
    //produces<std::vector<unsigned> >                (prefix_ + "stackId"        + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "barrel"         + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "outer"          + suffix_);
    //produces<std::vector<unsigned> >                (prefix_ + "subdet"         + suffix_);
    produces<std::vector<int> >                     (prefix_ + "col"            + suffix_);
    produces<std::vector<int> >                     (prefix_ + "row"            + suffix_);
    produces<std::vector<int> >                     (prefix_ + "adc"            + suffix_);
    produces<std::vector<int> >                     (prefix_ + "channel"        + suffix_);
    produces<std::vector<std::vector<unsigned> > >  (prefix_ + "trkIds"         + suffix_);
    produces<std::vector<std::vector<unsigned> > >  (prefix_ + "evtIds"         + suffix_);
    produces<std::vector<std::vector<int> > >       (prefix_ + "tpIds"          + suffix_);
    produces<std::vector<std::vector<unsigned> > >  (prefix_ + "cfPositions"    + suffix_);
    produces<std::vector<std::vector<unsigned> > >  (prefix_ + "simHitRefs"     + suffix_);
    produces<std::vector<std::vector<bool> > >      (prefix_ + "tofBins"        + suffix_);
    produces<std::vector<std::vector<float> > >     (prefix_ + "fractions"      + suffix_);
    //produces<std::vector<std::vector<float> > >     (prefix_ + "amplitudes"     + suffix_);
    produces<unsigned>                              (prefix_ + "size"           + suffix_);
}

void NTuplePixelDigis::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    /// Geometry setup
    edm::ESHandle<TrackerGeometry> geometryHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geometryHandle);
    theGeometry = geometryHandle.product();

    edm::ESHandle<StackedTrackerGeometry> stackedGeometryHandle;
    iSetup.get<StackedTrackerGeometryRecord>().get(stackedGeometryHandle);
    theStackedGeometry = stackedGeometryHandle.product();
    assert(theStackedGeometry->getCBC3MaxStubs() == 0);
}

void NTuplePixelDigis::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    std::auto_ptr<std::vector<float> >                  v_x           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_y           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_z           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_r           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_eta         (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_phi         (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_localx      (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_localy      (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_localz      (new std::vector<float>());
    std::auto_ptr<std::vector<unsigned> >               v_modId       (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_geoId       (new std::vector<unsigned>());
    //std::auto_ptr<std::vector<unsigned> >               v_stackId       (new std::vector<unsigned>());
    std::auto_ptr<std::vector<bool> >                   v_barrel        (new std::vector<bool>());
    std::auto_ptr<std::vector<bool> >                   v_outer         (new std::vector<bool>());
    //std::auto_ptr<std::vector<unsigned> >               v_subdet      (new std::vector<unsigned>());
    std::auto_ptr<std::vector<int> >                    v_col         (new std::vector<int>());
    std::auto_ptr<std::vector<int> >                    v_row         (new std::vector<int>());
    std::auto_ptr<std::vector<int> >                    v_adc         (new std::vector<int>());
    std::auto_ptr<std::vector<int> >                    v_channel     (new std::vector<int>());
    std::auto_ptr<std::vector<std::vector<unsigned> > > v_trkIds      (new std::vector<std::vector<unsigned> >());
    std::auto_ptr<std::vector<std::vector<unsigned> > > v_evtIds      (new std::vector<std::vector<unsigned> >());
    std::auto_ptr<std::vector<std::vector<int> > >      v_tpIds       (new std::vector<std::vector<int> >());
    std::auto_ptr<std::vector<std::vector<unsigned> > > v_cfPositions (new std::vector<std::vector<unsigned> >());
    std::auto_ptr<std::vector<std::vector<unsigned> > > v_simHitRefs  (new std::vector<std::vector<unsigned> >());
    std::auto_ptr<std::vector<std::vector<bool> > >     v_tofBins     (new std::vector<std::vector<bool> >());
    std::auto_ptr<std::vector<std::vector<float> > >    v_fractions   (new std::vector<std::vector<float> >());
    //std::auto_ptr<std::vector<std::vector<float> > >    v_amplitudes  (new std::vector<std::vector<float> >());
    std::auto_ptr<unsigned>                             v_size        (new unsigned(0));

    // _________________________________________________________________________
    typedef edm::DetSetVector<PixelDigi>                     dsv_digi;
    typedef edm::DetSet<PixelDigi>                           ds_digi;
    typedef edm::DetSetVector<PixelDigiSimLink>              dsv_digisimlink;
    typedef edm::DetSet<PixelDigiSimLink>                    ds_digisimlink;

    /// PixelDigi
    edm::Handle<dsv_digi> pixelDigis;
    if (inputTag_.encode() != "")
        iEvent.getByLabel(inputTag_, pixelDigis);

    /// PixelDigiSimLink
    edm::Handle<dsv_digisimlink> pixelDigiSimLinks;
    if (inputTag_.encode() != "" && !iEvent.isRealData())
        iEvent.getByLabel(inputTag_, pixelDigiSimLinks);

    /// Prepare a map of simTrack -> trackingParticle
    edm::Handle<TrackingParticleCollection> trackingParticleHandle;
    iEvent.getByLabel(inputTagTP_, trackingParticleHandle);

    MapTrackingParticles trkToTPMap;
    trkToTPMap.setup(trackingParticleHandle);

    /// Prepare a map of CFposition -> simHits
    MapMixedSimHits mixedSimHitMap;
    mixedSimHitMap.setup(simHitCollections_, iEvent);

    /// Prepare detId -> moduleId
    ModuleIdFunctor getModuleId;

    if (pixelDigis.isValid()) {
        edm::LogInfo("NTuplePixelDigis") << "Size: " << pixelDigis->size();

        unsigned n = 0;
        for (dsv_digi::const_iterator itv = pixelDigis->begin(); itv != pixelDigis->end(); ++itv) {
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
            const PixelTopology* topology = dynamic_cast<const PixelTopology*>(&(geomDetUnit->specificTopology()) );

            // This cut is as used in Geometry/TrackerGeometryBuilder/src/StackedTrackerGeometryBuilder.cc
            if (geomDetUnit->position().perp()<20.)
                continue;  // only outer tracker

            /// Module ID
            const unsigned moduleId = getModuleId(geoId);
            edm::LogInfo("NTuplePixelDigis") << "geoId: " << geoId.rawId() << " det: " << det << " subdet: " << subdet << " modId: " << moduleId << " size: " << itv->size();

            /// Stack member: inner sensor=0, outer sensor=1
            unsigned stackMember = ((geoId>>2)%2 == 0);

            for (ds_digi::const_iterator it = itv->begin(); it != itv->end(); ++it) {
                //if (!selector_(*it))
                //    continue;

                int row = it->row();
                int col = it->column();
                int channel = it->channel();  // = ((row << 11) | col)
                unsigned adc = it->adc();
                edm::LogInfo("NTuplePixelDigis") << "row: " << row << " col: " << col << " channel: " << channel << " adc: " << adc;

                MeasurementPoint mp((float) row + 0.5, (float) col + 0.5);
                const LocalPoint& localPosition = topology->localPosition(mp);
                const GlobalPoint& globalPosition = geomDetUnit->surface().toGlobal(localPosition);

                // Find simlinks
                std::vector<unsigned> trkIds;
                std::vector<unsigned> evtIds;
                std::vector<int>      tpIds;
                std::vector<unsigned> cfPositions;
                std::vector<unsigned> simHitRefs;
                std::vector<bool>     tofBins;
                std::vector<float>    fractions;
                //std::vector<float>    amplitudes;
                if (pixelDigiSimLinks.isValid() && pixelDigiSimLinks->find(geoId) != pixelDigiSimLinks->end()) {
                    const ds_digisimlink& simlink = (*pixelDigiSimLinks)[geoId];
                    for (ds_digisimlink::const_iterator itsim = simlink.data.begin(); itsim != simlink.data.end(); ++itsim) {
                        if ((unsigned) channel == itsim->channel()) {
                            trkIds     .push_back(itsim->SimTrackId());
                            evtIds     .push_back(itsim->eventId().rawId());
                            tpIds      .push_back(trkToTPMap.get(itsim->SimTrackId(), itsim->eventId()));
                            cfPositions.push_back(itsim->CFposition());
                            simHitRefs .push_back(mixedSimHitMap.get(subdet, itsim->TofBin(), itsim->CFposition()));
                            tofBins    .push_back(itsim->TofBin());
                            fractions  .push_back(itsim->fraction());
                            //amplitudes .push_back(itsim->amplitude());
                            //edm::LogInfo("NTuplePixelDigis") << "trkId: " << trkIds.back() << " evtId: " << evtIds.back() << " tpId: " << tpIds.back() << " cfPosition: " << cfPositions.back() << " simHitRef: " << simHitRefs.back() << " tofBin: " << tofBins.back() << " fraction: " << fractions.back();
                        }
                    }
                }

                v_x->push_back(globalPosition.x());
                v_y->push_back(globalPosition.y());
                v_z->push_back(globalPosition.z());
                v_r->push_back(globalPosition.perp());
                v_eta->push_back(globalPosition.eta());
                v_phi->push_back(globalPosition.phi());
                //v_localx->push_back(localPosition.x());
                //v_localy->push_back(localPosition.y());
                //v_localz->push_back(localPosition.z());
                v_modId->push_back(moduleId);
                v_geoId->push_back(geoId.rawId());
                //v_stackId->push_back(stackDetId.rawId());
                v_barrel->push_back(isBarrel);
                v_outer->push_back(stackMember);
                //v_subdet->push_back(subdet);
                v_col->push_back(col);
                v_row->push_back(row);
                v_adc->push_back(adc);
                v_channel->push_back(channel);
                v_trkIds->push_back(trkIds);
                v_evtIds->push_back(evtIds);
                v_tpIds->push_back(tpIds);
                v_cfPositions->push_back(cfPositions);
                v_simHitRefs->push_back(simHitRefs);
                v_tofBins->push_back(tofBins);
                v_fractions->push_back(fractions);
                //v_amplitudes->push_back(amplitudes);

                n++;
            }
        }
        *v_size = v_x->size();

    } else {
        edm::LogError("NTuplePixelDigis") << "Cannot get the product: " << inputTag_;
    }


    //__________________________________________________________________________
    iEvent.put(v_x             , prefix_ + "x"              + suffix_);
    iEvent.put(v_y             , prefix_ + "y"              + suffix_);
    iEvent.put(v_z             , prefix_ + "z"              + suffix_);
    iEvent.put(v_r             , prefix_ + "r"              + suffix_);
    iEvent.put(v_eta           , prefix_ + "eta"            + suffix_);
    iEvent.put(v_phi           , prefix_ + "phi"            + suffix_);
    //iEvent.put(v_localx        , prefix_ + "localx"         + suffix_);
    //iEvent.put(v_localy        , prefix_ + "localy"         + suffix_);
    //iEvent.put(v_localz        , prefix_ + "localz"         + suffix_);
    iEvent.put(v_modId         , prefix_ + "modId"          + suffix_);
    iEvent.put(v_geoId         , prefix_ + "geoId"          + suffix_);
    //iEvent.put(v_stackId       , prefix_ + "stackId"        + suffix_);
    iEvent.put(v_barrel        , prefix_ + "barrel"         + suffix_);
    iEvent.put(v_outer         , prefix_ + "outer"          + suffix_);
    //iEvent.put(v_subdet        , prefix_ + "subdet"         + suffix_);
    iEvent.put(v_col           , prefix_ + "col"            + suffix_);
    iEvent.put(v_row           , prefix_ + "row"            + suffix_);
    iEvent.put(v_adc           , prefix_ + "adc"            + suffix_);
    iEvent.put(v_channel       , prefix_ + "channel"        + suffix_);
    iEvent.put(v_trkIds        , prefix_ + "trkIds"         + suffix_);
    iEvent.put(v_evtIds        , prefix_ + "evtIds"         + suffix_);
    iEvent.put(v_tpIds         , prefix_ + "tpIds"          + suffix_);
    iEvent.put(v_cfPositions   , prefix_ + "cfPositions"    + suffix_);
    iEvent.put(v_simHitRefs    , prefix_ + "simHitRefs"     + suffix_);
    iEvent.put(v_tofBins       , prefix_ + "tofBins"        + suffix_);
    iEvent.put(v_fractions     , prefix_ + "fractions"      + suffix_);
    //iEvent.put(v_amplitudes    , prefix_ + "amplitudes"     + suffix_);
    iEvent.put(v_size          , prefix_ + "size"           + suffix_);
}
