#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTTClusters.h"

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/ModuleIdFunctor.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapTrackingParticles.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/MapPixelDigis.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "SimTracker/TrackTriggerAssociation/interface/TTClusterAssociationMap.h"


NTupleTTClusters::NTupleTTClusters(const edm::ParameterSet& iConfig) :
  inputTag_    (iConfig.getParameter<edm::InputTag>("inputTag")),
  inputTagMC_  (iConfig.getParameter<edm::InputTag>("inputTagMC")),
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
    //produces<std::vector<float> >                   (prefix_ + "localx"         + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "localy"         + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "localz"         + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "surfx"          + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "surfy"          + suffix_);
    //produces<std::vector<float> >                   (prefix_ + "surfz"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "modId"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "geoId"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "stackId"        + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "barrel"         + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "psmodule"       + suffix_);
    produces<std::vector<bool> >                    (prefix_ + "outer"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "width"          + suffix_);
    produces<std::vector<unsigned> >                (prefix_ + "ndigis"         + suffix_);
    produces<std::vector<std::vector<int> > >       (prefix_ + "digiChannels"   + suffix_);
    produces<std::vector<std::vector<unsigned> > >  (prefix_ + "digiRefs"       + suffix_);
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


void NTupleTTClusters::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
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

void NTupleTTClusters::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    std::auto_ptr<std::vector<float> >                  v_x             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_y             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_z             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_r             (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_eta           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_phi           (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_coordx        (new std::vector<float>());
    std::auto_ptr<std::vector<float> >                  v_coordy        (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_localx        (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_localy        (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_localz        (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_surfx         (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_surfy         (new std::vector<float>());
    //std::auto_ptr<std::vector<float> >                  v_surfz         (new std::vector<float>());
    std::auto_ptr<std::vector<unsigned> >               v_modId         (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_geoId         (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_stackId       (new std::vector<unsigned>());
    std::auto_ptr<std::vector<bool> >                   v_barrel        (new std::vector<bool>());
    std::auto_ptr<std::vector<bool> >                   v_psmodule      (new std::vector<bool>());
    std::auto_ptr<std::vector<bool> >                   v_outer         (new std::vector<bool>());
    std::auto_ptr<std::vector<unsigned> >               v_width         (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> >               v_ndigis        (new std::vector<unsigned>());
    std::auto_ptr<std::vector<std::vector<int> > >      v_digiChannels  (new std::vector<std::vector<int> >());
    std::auto_ptr<std::vector<std::vector<unsigned> > > v_digiRefs      (new std::vector<std::vector<unsigned> >());
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
    iEvent.getByLabel(inputTag_, pixelDigiTTClusters);

    /// MC truth association map
    edm::Handle<TTClusterAssociationMap<Ref_PixelDigi_> > mcAssocTTClusters;
    if (!iEvent.isRealData())
        iEvent.getByLabel(inputTagMC_, mcAssocTTClusters);

    /// Prepare detId -> moduleId
    ModuleIdFunctor getModuleId;

    // _________________________________________________________________________
    if (pixelDigiTTClusters.isValid()) {
        edm::LogInfo("NTupleClusters") << "Size: " << pixelDigiTTClusters->size();

        unsigned n = 0;
        for (dsv_clus::const_iterator itv = pixelDigiTTClusters->begin(); itv != pixelDigiTTClusters->end(); ++itv) {

            for (ds_clus::const_iterator it = itv->begin(); it != itv->end(); ++it) {
                //if (!selector_(*it))
                //    continue;

                const StackedTrackerDetId stackDetId(it->getDetId());

                bool isBarrel = stackDetId.isBarrel();
                bool isEndcap = stackDetId.isEndcap();
                bool isPSModule = theStackedGeometry->isPSModule(stackDetId);
                if (!isBarrel && !isEndcap) {
                    edm::LogError("NTupleClusters") << "Inconsistent isBarrel: " << isBarrel << " vs isEndcap: " << isEndcap;
                }
                unsigned stackMember = it->getStackMember();  // inner sensor=0, outer sensor=1
                if (stackMember != 0 && stackMember != 1) {
                    edm::LogError("NTupleClusters") << "Inconsistent stack number: " << stackMember;
                }

                /// Module ID
                const DetId geoId = theStackedGeometry->idToDet(stackDetId, stackMember)->geographicalId();
                const unsigned moduleId = getModuleId(geoId);
                edm::LogInfo("NTupleTTClusters") << "geoId: " << geoId.rawId() << " modId: " << moduleId;

                /// Positions
                const MeasurementPoint& localCoord     = it->findAverageLocalCoordinates();
                const LocalPoint&       localPosition  = theStackedGeometry->findAverageLocalPosition(&(*it));
                const GlobalPoint&      globalPosition = theStackedGeometry->findAverageGlobalPosition(&(*it));
                edm::LogInfo("NTupleTTStubs") << "localCoord: " << localCoord.x() << "," << localCoord.y();

                /// Topology
                //const GeomDetUnit* geoUnit = theStackedGeometry->idToDetUnit(stackDetId, stackMember);
                //const PixelGeomDetUnit* pixUnit = dynamic_cast<const PixelGeomDetUnit*>(geoUnit);
                //const PixelTopology* pixTopo = dynamic_cast<const PixelTopology*>(&(pixUnit->specificTopology()) );

                //const Surface::PositionType& surfPosition = pixUnit->position();
                //const Surface::RotationType& surfRotation = pixUnit->rotation();

                /// Find cluster width
                unsigned width = it->findWidth();

                /// Find digis
                const std::vector<int>& theCols = it->getCols();
                const std::vector<int>& theRows = it->getRows();
                assert(theCols.size() != 0 && theRows.size() != 0 && theCols.size() == theRows.size());

                unsigned ndigis = theCols.size();
                std::vector<int> digiChannels;
                std::vector<unsigned> digiRefs;
                for (unsigned idigi=0; idigi<ndigis; ++idigi) {
                    const int channel = PixelDigi::pixelToChannel(theRows.at(idigi), theCols.at(idigi));
                    const unsigned ref = digiMap.size() > 0 ? digiMap.get(geoId.rawId(), channel) : 0;
                    digiChannels.push_back(channel);
                    digiRefs.push_back(ref);
                }


                v_x->push_back(globalPosition.x());                   // sviret/HL_LHC: CLUS_x
                v_y->push_back(globalPosition.y());                   // sviret/HL_LHC: CLUS_y
                v_z->push_back(globalPosition.z());                   // sviret/HL_LHC: CLUS_z
                v_r->push_back(globalPosition.perp());
                v_eta->push_back(globalPosition.eta());
                v_phi->push_back(globalPosition.phi());
                v_coordx->push_back(localCoord.x());                  // sviret/HL_LHC: CLUS_strip
                v_coordy->push_back(localCoord.y());                  // sviret/HL_LHC: CLUS_seg
                //v_localx->push_back(localPosition.x());
                //v_localy->push_back(localPosition.y());
                //v_localz->push_back(localPosition.z());
                //v_surfx->push_back(surfPosition.x());
                //v_surfy->push_back(surfPosition.y());
                //v_surfz->push_back(surfPosition.z());
                v_modId->push_back(moduleId);
                v_geoId->push_back(geoId.rawId());
                v_stackId->push_back(stackDetId.rawId());
                v_barrel->push_back(isBarrel);
                v_psmodule->push_back(isPSModule);
                v_outer->push_back(stackMember);
                v_width->push_back(width);                            // sviret/HL_LHC: CLUS_nstrips
                v_ndigis->push_back(ndigis);
                v_digiChannels->push_back(digiChannels);
                v_digiRefs->push_back(digiRefs);

                // Set to dummy values first
                v_isGenuine->push_back(false);
                v_isUnknown->push_back(false);
                v_isCombinatoric->push_back(false);
                v_tpId->push_back(-1);                                // sviret/HL_LHC: CLUS_tp
                v_pdgId->push_back(-99);                              // sviret/HL_LHC: CLUS_pdgID
                v_simPt->push_back(-99.);
                v_simEta->push_back(-99.);
                v_simPhi->push_back(-99.);
                v_tpIds->push_back(std::vector<int>());
                //v_fractions->push_back(std::vector<float>());

                /// Retrieve MC association
                if (mcAssocTTClusters.isValid()) {
                    ref_clus iref = edmNew::makeRefTo(pixelDigiTTClusters, it);
                    v_isGenuine->back() = mcAssocTTClusters->isGenuine(iref);
                    v_isUnknown->back() = mcAssocTTClusters->isUnknown(iref);
                    v_isCombinatoric->back() = mcAssocTTClusters->isCombinatoric(iref);
                    const edm::Ptr<TrackingParticle> tpptr = mcAssocTTClusters->findTrackingParticlePtr(iref);
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
                    std::vector<int> tpIds;

                    const std::vector<Ref_PixelDigi_>& theHits = it->getHits();
                    assert(theHits.size() == theCols.size());

                    if (pixelDigiSimLinks->find(geoId) != pixelDigiSimLinks->end()) {
                        const ds_digisimlink& simlink = (*pixelDigiSimLinks)[geoId];
                        for (ds_digisimlink::const_iterator itsim = simlink.data.begin(); itsim != simlink.data.end(); ++itsim) {
                            for (unsigned ihit=0; ihit<theHits.size(); ++ihit) {
                                if ((unsigned) theHits.at(ihit)->channel() == itsim->channel()) {
                                    if (itsim->fraction() > 0.3)  // 0.3 is arbitrary
                                        tpIds.push_back(trkToTPMap.get(itsim->SimTrackId(), itsim->eventId()));
                                }
                            }
                        }
                    }

                    // Uniquify
                    std::sort(tpIds.begin(), tpIds.end());
                    tpIds.erase(std::unique(tpIds.begin(), tpIds.end()), tpIds.end());
                    v_tpIds->back() = tpIds;
                }

                n++;
            }
        }
        *v_size = v_x->size();

    } else {
        edm::LogError("NTupleClusters") << "Cannot get the product: " << inputTag_;
    }


    //__________________________________________________________________________
    iEvent.put(v_x             , prefix_ + "x"              + suffix_);
    iEvent.put(v_y             , prefix_ + "y"              + suffix_);
    iEvent.put(v_z             , prefix_ + "z"              + suffix_);
    iEvent.put(v_r             , prefix_ + "r"              + suffix_);
    iEvent.put(v_eta           , prefix_ + "eta"            + suffix_);
    iEvent.put(v_phi           , prefix_ + "phi"            + suffix_);
    iEvent.put(v_coordx        , prefix_ + "coordx"         + suffix_);
    iEvent.put(v_coordy        , prefix_ + "coordy"         + suffix_);
    //iEvent.put(v_localx        , prefix_ + "localx"         + suffix_);
    //iEvent.put(v_localy        , prefix_ + "localy"         + suffix_);
    //iEvent.put(v_localz        , prefix_ + "localz"         + suffix_);
    //iEvent.put(v_surfx         , prefix_ + "surfx"          + suffix_);
    //iEvent.put(v_surfy         , prefix_ + "surfy"          + suffix_);
    //iEvent.put(v_surfz         , prefix_ + "surfz"          + suffix_);
    iEvent.put(v_modId         , prefix_ + "modId"          + suffix_);
    iEvent.put(v_geoId         , prefix_ + "geoId"          + suffix_);
    iEvent.put(v_stackId       , prefix_ + "stackId"        + suffix_);
    iEvent.put(v_barrel        , prefix_ + "barrel"         + suffix_);
    iEvent.put(v_psmodule      , prefix_ + "psmodule"       + suffix_);
    iEvent.put(v_outer         , prefix_ + "outer"          + suffix_);
    iEvent.put(v_width         , prefix_ + "width"          + suffix_);
    iEvent.put(v_ndigis        , prefix_ + "ndigis"         + suffix_);
    iEvent.put(v_digiChannels  , prefix_ + "digiChannels"   + suffix_);
    iEvent.put(v_digiRefs      , prefix_ + "digiRefs"       + suffix_);
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
