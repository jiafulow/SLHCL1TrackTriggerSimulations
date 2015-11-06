#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTTStubs.h"

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/ModuleIdFunctor.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTrackingParticles.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapPixelDigis.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTTClusters.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "SimTracker/TrackTriggerAssociation/interface/TTClusterAssociationMap.h"
#include "SimTracker/TrackTriggerAssociation/interface/TTStubAssociationMap.h"


NTupleTTStubs::NTupleTTStubs(const edm::ParameterSet& iConfig) :
  inputTag_    (iConfig.getParameter<edm::InputTag>("inputTag")),
  inputTagMC_  (iConfig.getParameter<edm::InputTag>("inputTagMC")),
  inputTagClus_(iConfig.getParameter<edm::InputTag>("inputTagClus")),
  inputTagDigi_(iConfig.getParameter<edm::InputTag>("inputTagDigi")),
  inputTagTP_  (iConfig.getParameter<edm::InputTag>("inputTagTP")),
  prefix_      (iConfig.getParameter<std::string>("prefix")),
  suffix_      (iConfig.getParameter<std::string>("suffix")),
  selector_    (iConfig.existsAs<std::string>("cut") ? iConfig.getParameter<std::string>("cut") : "", true),
  maxN_        (iConfig.getParameter<unsigned>("maxN")) {

    // Remember r is rho in cylindrical coordinate system
    produces<std::vector<float> >                   (prefix_ + "x"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "y"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "z"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "r"              + suffix_);
    produces<std::vector<float> >                   (prefix_ + "eta"            + suffix_);
    produces<std::vector<float> >                   (prefix_ + "phi"            + suffix_);
    produces<std::vector<float> >                   (prefix_ + "coordx"         + suffix_);
    produces<std::vector<float> >                   (prefix_ + "coordy"         + suffix_);
    produces<std::vector<float> >                   (prefix_ + "dirx"           + suffix_);
    produces<std::vector<float> >                   (prefix_ + "diry"           + suffix_);
    produces<std::vector<float> >                   (prefix_ + "dirz"           + suffix_);
    produces<std::vector<float> >                   (prefix_ + "roughPt"        + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "modId"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "geoId0"         + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "geoId1"         + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "stackId"        + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "barrel"         + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "psmodule"       + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "clusRef0"       + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "clusRef1"       + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "clusWidth0"     + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "clusWidth1"     + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "ndigis"         + suffix_);
    produces<std::vector<std::vector<int> > >       (prefix_ + "digiChannels"   + suffix_);
    produces<std::vector<std::vector<unsigned> > >  (prefix_ + "digiRefs"       + suffix_);
    produces<std::vector<float> >                   (prefix_ + "trigDist"       + suffix_);
    produces<std::vector<float> >                   (prefix_ + "trigOffset"     + suffix_);
    produces<std::vector<float> >                   (prefix_ + "trigPos"        + suffix_);
    produces<std::vector<float> >                   (prefix_ + "trigBend"       + suffix_);
    produces<std::vector<float> >                   (prefix_ + "separation"     + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "isGenuine"      + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "isUnknown"      + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "isCombinatoric" + suffix_);
    produces<std::vector<int> >                     (prefix_ + "tpId"           + suffix_);
    produces<std::vector<int> >                     (prefix_ + "pdgId"          + suffix_);
    produces<std::vector<float> >                   (prefix_ + "simPt"          + suffix_);
    produces<std::vector<float> >                   (prefix_ + "simEta"         + suffix_);
    produces<std::vector<float> >                   (prefix_ + "simPhi"         + suffix_);
    produces<std::vector<std::vector<int> > >       (prefix_ + "tpIds"          + suffix_);
    //produces<std::vector<std::vector<float> > >     (prefix_ + "fractions"      + suffix_);
    produces<unsigned>                              (prefix_ + "size"           + suffix_);
}

void NTupleTTStubs::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    /// Geometry setup
    edm::ESHandle<TrackerGeometry> geometryHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geometryHandle);
    theGeometry = geometryHandle.product();

    edm::ESHandle<StackedTrackerGeometry> stackedGeometryHandle;
    iSetup.get<StackedTrackerGeometryRecord>().get(stackedGeometryHandle);
    theStackedGeometry = stackedGeometryHandle.product();
    assert(theStackedGeometry->getCBC3MaxStubs() == 0);

    /// Magnetic field setup
    edm::ESHandle<MagneticField> magneticFieldHandle;
    iSetup.get<IdealMagneticFieldRecord>().get(magneticFieldHandle);
    theMagneticField = magneticFieldHandle.product();
}

void NTupleTTStubs::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    std::auto_ptr<std::vector<float> >                  v_x             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_y             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_z             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_r             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_eta           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_phi           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_coordx        (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_coordy        (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_dirx          (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_diry          (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_dirz          (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_roughPt       (new std::vector<float>());
    std::auto_ptr<std::vector<unsigned> >               v_modId         (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_geoId0        (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_geoId1        (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_stackId       (new std::vector<unsigned>());
    std::auto_ptr<std::vector<bool> >                   v_barrel        (new std::vector<bool>());
    std::auto_ptr<std::vector<bool> >                   v_psmodule      (new std::vector<bool>());
    std::auto_ptr<std::vector<unsigned> >               v_clusRef0      (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_clusRef1      (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_clusWidth0    (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_clusWidth1    (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_ndigis        (new std::vector<unsigned>());
    std::auto_ptr<std::vector<std::vector<int> > >      v_digiChannels  (new std::vector<std::vector<int> >());
    std::auto_ptr<std::vector<std::vector<unsigned> > > v_digiRefs      (new std::vector<std::vector<unsigned> >());
    std::auto_ptr<std::vector<float> >                  v_trigDist      (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_trigOffset    (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_trigPos       (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_trigBend      (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_separation    (new std::vector<float>());
    std::auto_ptr<std::vector<bool> >                   v_isGenuine     (new std::vector<bool>());
    std::auto_ptr<std::vector<bool> >                   v_isUnknown     (new std::vector<bool>());
    std::auto_ptr<std::vector<bool> >                   v_isCombinatoric(new std::vector<bool>());
    std::auto_ptr<std::vector<int> >                    v_tpId          (new std::vector<int>());
    std::auto_ptr<std::vector<int> >                    v_pdgId         (new std::vector<int>());
    std::auto_ptr<std::vector<float> >                  v_simPt         (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_simEta        (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_simPhi        (new std::vector<float>());
    std::auto_ptr<std::vector<std::vector<int> > >      v_tpIds         (new std::vector<std::vector<int> >());
    //std::auto_ptr<std::vector<std::vector<float> > >    v_fractions     (new std::vector<std::vector<float> >());
    std::auto_ptr<unsigned>                             v_size          (new unsigned(0));

    // _________________________________________________________________________
    typedef edm::DetSetVector<PixelDigi>                     dsv_digi;
    typedef edm::DetSet<PixelDigi>                           ds_digi;
    typedef edm::DetSetVector<PixelDigiSimLink>              dsv_digisimlink;
    typedef edm::DetSet<PixelDigiSimLink>                    ds_digisimlink;
    typedef edmNew::DetSetVector<TTCluster<Ref_PixelDigi_> > dsv_clus;
    typedef edmNew::DetSet<TTCluster<Ref_PixelDigi_> >       ds_clus;
    typedef edm::Ref<dsv_clus, TTCluster<Ref_PixelDigi_> >   ref_clus;
    typedef TTClusterAssociationMap<Ref_PixelDigi_>          assocmap_clus;
    typedef edmNew::DetSetVector<TTStub<Ref_PixelDigi_> >    dsv_stub;
    typedef edmNew::DetSet<TTStub<Ref_PixelDigi_> >          ds_stub;
    typedef edm::Ref<dsv_stub, TTStub<Ref_PixelDigi_> >      ref_stub;
    typedef TTStubAssociationMap<Ref_PixelDigi_>             assocmap_stub;

    /// PixelDigi
    edm::Handle<dsv_digi> pixelDigis;
    if (inputTagDigi_.encode() != "")
        iEvent.getByLabel(inputTagDigi_, pixelDigis);

    MapPixelDigis digiMap;
    digiMap.setup(pixelDigis, theGeometry);

    /// PixelDigiSimLink
    edm::Handle<dsv_digisimlink> pixelDigiSimLinks;
    if (inputTagDigi_.encode() != "" && !iEvent.isRealData())
        iEvent.getByLabel(inputTagDigi_, pixelDigiSimLinks);

    /// Prepare a map of simTrack -> trackingParticle
    edm::Handle<TrackingParticleCollection> trackingParticleHandle;
    iEvent.getByLabel(inputTagTP_, trackingParticleHandle);

    MapTrackingParticles trkToTPMap;
    trkToTPMap.setup(trackingParticleHandle);

    /// TTCluster
    edm::Handle<dsv_clus> pixelDigiTTClusters;
    iEvent.getByLabel(inputTagClus_, pixelDigiTTClusters);

    MapTTClusters clusMap;
    clusMap.setup(pixelDigiTTClusters);

    /// TTStub
    edm::Handle<dsv_stub> pixelDigiTTStubs;
    iEvent.getByLabel(inputTag_, pixelDigiTTStubs);

    /// MC truth association map
    edm::Handle<assocmap_stub> mcAssocTTStubs;
    if (!iEvent.isRealData())
        iEvent.getByLabel(inputTagMC_, mcAssocTTStubs);

    /// Prepare detId -> moduleId
    ModuleIdFunctor getModuleId;

    // _________________________________________________________________________
    if (pixelDigiTTStubs.isValid()) {
        edm::LogInfo("NTupleStubs") << "Size: " << pixelDigiTTStubs->size();

        unsigned n = 0;
        for (dsv_stub::const_iterator itv = pixelDigiTTStubs->begin(); itv != pixelDigiTTStubs->end(); ++itv) {

            for (ds_stub::const_iterator it = itv->begin(); it != itv->end(); ++it) {
                //if (!selector_(*it))
                //    continue;

                const StackedTrackerDetId stackDetId(it->getDetId());

                bool isBarrel = stackDetId.isBarrel();
                bool isEndcap = stackDetId.isEndcap();
                bool isPSModule = theStackedGeometry->isPSModule(stackDetId);
                if (!isBarrel && !isEndcap) {
                    edm::LogError("NTupleStubs") << "Inconsistent isBarrel: " << isBarrel << " vs isEndcap: " << isEndcap;
                }
                const std::vector<ref_clus>& clusterRefs = it->getClusterRefs();
                if (clusterRefs.size() != 2) {
                    edm::LogError("NTupleStubs") << "# clusters != 2: " << clusterRefs.size();
                }
                ref_clus cluster0 = clusterRefs.at(0);
                ref_clus cluster1 = clusterRefs.at(1);
                const unsigned myClusRef0 = clusMap.get(cluster0);
                const unsigned myClusRef1 = clusMap.get(cluster1);

                /// Module ID
                const DetId geoId0 = theStackedGeometry->idToDet(stackDetId, 0)->geographicalId();
                const DetId geoId1 = theStackedGeometry->idToDet(stackDetId, 1)->geographicalId();
                const unsigned moduleId0 = getModuleId(geoId0);
                const unsigned moduleId1 = getModuleId(geoId1);
                assert(moduleId0 == moduleId1);
                edm::LogInfo("NTupleTTStubs") << "geoId0: " << geoId0.rawId() << " geoId1: " << geoId1.rawId() << " modId0: " << moduleId0 << " modId1: " << moduleId1;

                /// Positions, directions
                const GlobalPoint&      globalPosition  = theStackedGeometry->findGlobalPosition(&(*it));
                const GlobalVector&     globalDirection = theStackedGeometry->findGlobalDirection(&(*it));
                const MeasurementPoint& localCoord0     = cluster0->findAverageLocalCoordinates();
                const MeasurementPoint& localCoord1     = cluster1->findAverageLocalCoordinates();
                double magfieldStrength = theMagneticField->inTesla(GlobalPoint(0,0,0)).z();
                double roughPt = theStackedGeometry->findRoughPt(magfieldStrength, &(*it));
                edm::LogInfo("NTupleTTStubs") << "localCoord0: " << localCoord0.x() << "," << localCoord0.y() << " localCoord1: " << localCoord1.x() << "," << localCoord1.y();

                /// Topology
                const GeomDetUnit* geoUnit0 = theStackedGeometry->idToDetUnit(stackDetId, 0);
                const GeomDetUnit* geoUnit1 = theStackedGeometry->idToDetUnit(stackDetId, 1);
                const PixelGeomDetUnit* pixUnit0 = dynamic_cast<const PixelGeomDetUnit*>(geoUnit0);
                const PixelGeomDetUnit* pixUnit1 = dynamic_cast<const PixelGeomDetUnit*>(geoUnit1);
                const Surface::PositionType& surfPosition0 = pixUnit0->position();
                const Surface::PositionType& surfPosition1 = pixUnit1->position();
                double separation = (moduleId0 < 110000) ? surfPosition1.perp() - surfPosition0.perp() : ((moduleId0 < 180000) ? surfPosition1.z() - surfPosition0.z() : surfPosition0.z() - surfPosition1.z());

                /// Find cluster widths
                unsigned clusWidth0 = cluster0->findWidth();
                unsigned clusWidth1 = cluster1->findWidth();

                /// Find digis
                const std::vector<int>& theCols = cluster0->getCols();
                const std::vector<int>& theRows = cluster0->getRows();
                assert(theCols.size() != 0 && theRows.size() != 0 && theCols.size() == theRows.size());

                unsigned ndigis = theCols.size();
                std::vector<int> digiChannels;
                std::vector<unsigned> digiRefs;
                for (unsigned idigi=0; idigi<ndigis; ++idigi) {
                    const int channel = PixelDigi::pixelToChannel(theRows.at(idigi), theCols.at(idigi));
                    const unsigned ref = digiMap.size() > 0 ? digiMap.get(geoId0.rawId(), channel) : 0;
                    digiChannels.push_back(channel);
                    digiRefs.push_back(ref);
                }


                v_x->push_back(globalPosition.x());                   // sviret/HL_LHC: STUB_x
                v_y->push_back(globalPosition.y());                   // sviret/HL_LHC: STUB_y
                v_z->push_back(globalPosition.z());                   // sviret/HL_LHC: STUB_z
                v_r->push_back(globalPosition.perp());
                v_eta->push_back(globalPosition.eta());
                v_phi->push_back(globalPosition.phi());
                v_coordx->push_back(localCoord0.x());                 // sviret/HL_LHC: STUB_strip
                v_coordy->push_back(localCoord0.y());                 // sviret/HL_LHC: STUB_seg
                v_dirx->push_back(globalDirection.x());
                v_diry->push_back(globalDirection.y());
                v_dirz->push_back(globalDirection.z());
                v_roughPt->push_back(roughPt);                        // sviret/HL_LHC: STUB_pt
                v_modId->push_back(moduleId0);
                v_geoId0->push_back(geoId0.rawId());
                v_geoId1->push_back(geoId1.rawId());
                v_stackId->push_back(stackDetId.rawId());
                v_barrel->push_back(isBarrel);
                v_psmodule->push_back(isPSModule);
                v_clusRef0->push_back(myClusRef0);
                v_clusRef1->push_back(myClusRef1);
                v_clusWidth0->push_back(clusWidth0);
                v_clusWidth1->push_back(clusWidth1);
                v_ndigis->push_back(ndigis);
                v_digiChannels->push_back(digiChannels);
                v_digiRefs->push_back(digiRefs);
                v_trigDist->push_back(it->getTriggerDisplacement());  //                            (in full-strip units)
                v_trigOffset->push_back(it->getTriggerOffset());      // sviret/HL_LHC: STUB_cor    (in full-strip units)
                v_trigPos->push_back(it->getTriggerPosition());       //                            (in full-strip units)
                v_trigBend->push_back(it->getTriggerBend());          // sviret/HL_LHC: STUB_deltas (in full-strip units)
                v_separation->push_back(separation);

                // Set to dummy values first
                v_isGenuine->push_back(false);
                v_isUnknown->push_back(false);
                v_isCombinatoric->push_back(false);
                v_tpId->push_back(-1);                                // sviret/HL_LHC: STUB_tp
                v_pdgId->push_back(-99);                              // sviret/HL_LHC: STUB_pdgID
                v_simPt->push_back(-99.);
                v_simEta->push_back(-99.);
                v_simPhi->push_back(-99.);
                v_tpIds->push_back(std::vector<int>());
                //v_fractions->push_back(std::vector<float>());

                /// Retrieve MC association
                if (mcAssocTTStubs.isValid()) {
                    ref_stub iref = edmNew::makeRefTo(pixelDigiTTStubs, it);
                    v_isGenuine->back() = mcAssocTTStubs->isGenuine(iref);
                    v_isUnknown->back() = mcAssocTTStubs->isUnknown(iref);
                    v_isCombinatoric->back() = mcAssocTTStubs->isCombinatoric(iref);
                    const edm::Ptr<TrackingParticle> tpptr = mcAssocTTStubs->findTrackingParticlePtr(iref);
                    if (tpptr.isNonnull()) {
                        assert(v_isGenuine->back() == true);
                        v_tpId->back() = tpptr.key();
                        v_pdgId->back() = tpptr->pdgId();
                        v_simPt->back() = tpptr->pt();
                        v_simEta->back() = tpptr->eta();
                        v_simPhi->back() = tpptr->phi();
                    }
                }

                /// Tracking particle association
                if (pixelDigis.isValid() && pixelDigiSimLinks.isValid()) {
                    std::vector<int> tpIds0;
                    std::vector<int> tpIds1;

                    const std::vector<Ref_PixelDigi_>& theHits0 = cluster0->getHits();
                    assert(theHits0.size() == theCols.size());

                    if (pixelDigiSimLinks->find(geoId0) != pixelDigiSimLinks->end()) {
                        const ds_digisimlink& simlink0 = (*pixelDigiSimLinks)[geoId0];
                        for (ds_digisimlink::const_iterator itsim = simlink0.data.begin(); itsim != simlink0.data.end(); ++itsim) {
                            for (unsigned ihit=0; ihit<theHits0.size(); ++ihit) {
                                if ((unsigned) theHits0.at(ihit)->channel() == itsim->channel()) {
                                    if (itsim->fraction() > 0.3)  // 0.3 is arbitrary
                                        tpIds0.push_back(trkToTPMap.get(itsim->SimTrackId(), itsim->eventId()));
                                }
                            }
                        }
                    }

                    const std::vector<Ref_PixelDigi_>& theHits1 = cluster1->getHits();

                    if (pixelDigiSimLinks->find(geoId1) != pixelDigiSimLinks->end()) {
                        const ds_digisimlink& simlink1 = (*pixelDigiSimLinks)[geoId1];
                        for (ds_digisimlink::const_iterator itsim = simlink1.data.begin(); itsim != simlink1.data.end(); ++itsim) {
                            for (unsigned ihit=0; ihit<theHits1.size(); ++ihit) {
                                if ((unsigned) theHits1.at(ihit)->channel() == itsim->channel()) {
                                    if (itsim->fraction() > 0.3)  // 0.3 is arbitrary
                                        tpIds1.push_back(trkToTPMap.get(itsim->SimTrackId(), itsim->eventId()));
                                }
                            }
                        }
                    }

                    // Find common elements in two vectors
                    std::vector<int> tpIds;
                    std::sort(tpIds0.begin(), tpIds0.end());
                    std::sort(tpIds1.begin(), tpIds1.end());
                    tpIds0.erase(std::unique(tpIds0.begin(), tpIds0.end()), tpIds0.end());
                    tpIds1.erase(std::unique(tpIds1.begin(), tpIds1.end()), tpIds1.end());
                    std::set_intersection(tpIds0.begin(), tpIds0.end(), tpIds1.begin(), tpIds1.end(),
                                          std::back_inserter(tpIds));
                    v_tpIds->back() = tpIds;
                }

                n++;
            }
        }
        *v_size = v_x->size();

    } else {
        edm::LogError("NTupleStubs") << "Cannot get the product: " << inputTag_;
    }


    // _________________________________________________________________________
    iEvent.put(v_x             , prefix_ + "x"              + suffix_);
    iEvent.put(v_y             , prefix_ + "y"              + suffix_);
    iEvent.put(v_z             , prefix_ + "z"              + suffix_);
    iEvent.put(v_r             , prefix_ + "r"              + suffix_);
    iEvent.put(v_eta           , prefix_ + "eta"            + suffix_);
    iEvent.put(v_phi           , prefix_ + "phi"            + suffix_);
    iEvent.put(v_coordx        , prefix_ + "coordx"         + suffix_);
    iEvent.put(v_coordy        , prefix_ + "coordy"         + suffix_);
    iEvent.put(v_dirx          , prefix_ + "dirx"           + suffix_);
    iEvent.put(v_diry          , prefix_ + "diry"           + suffix_);
    iEvent.put(v_dirz          , prefix_ + "dirz"           + suffix_);
    iEvent.put(v_roughPt       , prefix_ + "roughPt"        + suffix_);
    iEvent.put(v_modId         , prefix_ + "modId"          + suffix_);
    iEvent.put(v_geoId0        , prefix_ + "geoId0"         + suffix_);
    iEvent.put(v_geoId1        , prefix_ + "geoId1"         + suffix_);
    iEvent.put(v_stackId       , prefix_ + "stackId"        + suffix_);
    iEvent.put(v_barrel        , prefix_ + "barrel"         + suffix_);
    iEvent.put(v_psmodule      , prefix_ + "psmodule"       + suffix_);
    iEvent.put(v_clusRef0      , prefix_ + "clusRef0"       + suffix_);
    iEvent.put(v_clusRef1      , prefix_ + "clusRef1"       + suffix_);
    iEvent.put(v_clusWidth0    , prefix_ + "clusWidth0"     + suffix_);
    iEvent.put(v_clusWidth1    , prefix_ + "clusWidth1"     + suffix_);
    iEvent.put(v_ndigis        , prefix_ + "ndigis"         + suffix_);
    iEvent.put(v_digiChannels  , prefix_ + "digiChannels"   + suffix_);
    iEvent.put(v_digiRefs      , prefix_ + "digiRefs"       + suffix_);
    iEvent.put(v_trigDist      , prefix_ + "trigDist"       + suffix_);
    iEvent.put(v_trigOffset    , prefix_ + "trigOffset"     + suffix_);
    iEvent.put(v_trigPos       , prefix_ + "trigPos"        + suffix_);
    iEvent.put(v_trigBend      , prefix_ + "trigBend"       + suffix_);
    iEvent.put(v_separation    , prefix_ + "separation"     + suffix_);
    iEvent.put(v_isGenuine     , prefix_ + "isGenuine"      + suffix_);
    iEvent.put(v_isUnknown     , prefix_ + "isUnknown"      + suffix_);
    iEvent.put(v_isCombinatoric, prefix_ + "isCombinatoric" + suffix_);
    iEvent.put(v_tpId          , prefix_ + "tpId"           + suffix_);
    iEvent.put(v_pdgId         , prefix_ + "pdgId"          + suffix_);
    iEvent.put(v_simPt         , prefix_ + "simPt"          + suffix_);
    iEvent.put(v_simEta        , prefix_ + "simEta"         + suffix_);
    iEvent.put(v_simPhi        , prefix_ + "simPhi"         + suffix_);
    iEvent.put(v_tpIds         , prefix_ + "tpIds"          + suffix_);
    //iEvent.put(v_fractions     , prefix_ + "fractions"      + suffix_);
    iEvent.put(v_size          , prefix_ + "size"           + suffix_);
}
