#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTuplePSimHits.h"

#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"


NTuplePSimHits::NTuplePSimHits(const edm::ParameterSet& iConfig) :
  inputTag_(iConfig.getParameter<edm::InputTag>("inputTag")),
  prefix_  (iConfig.getParameter<std::string>("prefix")),
  suffix_  (iConfig.getParameter<std::string>("suffix")),
  selector_(iConfig.existsAs<std::string>("cut") ? iConfig.getParameter<std::string>("cut") : "", true),
  maxN_    (iConfig.getParameter<unsigned>("maxN")) {

    produces<std::vector<float> >    (prefix_ + "localx"       + suffix_);
    produces<std::vector<float> >    (prefix_ + "localy"       + suffix_);
    produces<std::vector<float> >    (prefix_ + "localz"       + suffix_);
    produces<std::vector<float> >    (prefix_ + "pabs"         + suffix_);
    produces<std::vector<float> >    (prefix_ + "energyLoss"   + suffix_);
    produces<std::vector<float> >    (prefix_ + "thetaAtEntry" + suffix_);
    produces<std::vector<float> >    (prefix_ + "phiAtEntry"   + suffix_);
    produces<std::vector<float> >    (prefix_ + "tof"          + suffix_);
    produces<std::vector<int> >      (prefix_ + "particleType" + suffix_);
    produces<std::vector<int> >      (prefix_ + "processType"  + suffix_);
    produces<std::vector<unsigned> > (prefix_ + "detUnitId"    + suffix_);
    produces<std::vector<unsigned> > (prefix_ + "trkId"        + suffix_);
    produces<std::vector<unsigned> > (prefix_ + "evtId"        + suffix_);
    produces<unsigned>               (prefix_ + "size"         + suffix_);
}

void NTuplePSimHits::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    std::auto_ptr<std::vector<float> >    v_localx      (new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_localy      (new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_localz      (new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_pabs        (new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_energyLoss  (new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_thetaAtEntry(new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_phiAtEntry  (new std::vector<float>());
    std::auto_ptr<std::vector<float> >    v_tof         (new std::vector<float>());
    std::auto_ptr<std::vector<int> >      v_particleType(new std::vector<int>());
    std::auto_ptr<std::vector<int> >      v_processType (new std::vector<int>());
    std::auto_ptr<std::vector<unsigned> > v_detUnitId   (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> > v_trkId       (new std::vector<unsigned>());
    std::auto_ptr<std::vector<unsigned> > v_evtId       (new std::vector<unsigned>());
    std::auto_ptr<unsigned>               v_size        (new unsigned(0));

    //__________________________________________________________________________
    if (!iEvent.isRealData()) {
        edm::Handle<edm::PSimHitContainer> simhits;
        iEvent.getByLabel(inputTag_, simhits);

        if (simhits.isValid()) {
            edm::LogInfo("NTuplePSimHits") << "Size: " << simhits->size();

            unsigned n = 0;
            for (edm::PSimHitContainer::const_iterator it = simhits->begin(); it != simhits->end(); ++it) {
                if (n >= maxN_)
                    break;
                if (!selector_(*it))
                    continue;

                // Fill the vectors
                const Local3DPoint& localPosition = it->localPosition();
                v_localx->push_back(localPosition.x());
                v_localy->push_back(localPosition.y());
                v_localz->push_back(localPosition.z());
                v_pabs->push_back(it->pabs());
                v_energyLoss->push_back(it->energyLoss());
                v_thetaAtEntry->push_back(it->thetaAtEntry());
                v_phiAtEntry->push_back(it->phiAtEntry());
                v_tof->push_back(it->tof());
                v_particleType->push_back(it->particleType());
                v_processType->push_back(it->processType());
                v_detUnitId->push_back(it->detUnitId());
                v_trkId->push_back(it->trackId());
                v_evtId->push_back(it->eventId().rawId());

                n++;
            }
            *v_size = v_localx->size();

        } else {
            edm::LogError("NTuplePSimHits") << "Cannot get the product: " << inputTag_;
        }
    }

    //__________________________________________________________________________
    iEvent.put(v_localx      , prefix_ + "localx"       + suffix_);
    iEvent.put(v_localy      , prefix_ + "localy"       + suffix_);
    iEvent.put(v_localz      , prefix_ + "localz"       + suffix_);
    iEvent.put(v_pabs        , prefix_ + "pabs"         + suffix_);
    iEvent.put(v_energyLoss  , prefix_ + "energyLoss"   + suffix_);
    iEvent.put(v_thetaAtEntry, prefix_ + "thetaAtEntry" + suffix_);
    iEvent.put(v_phiAtEntry  , prefix_ + "phiAtEntry"   + suffix_);
    iEvent.put(v_tof         , prefix_ + "tof"          + suffix_);
    iEvent.put(v_particleType, prefix_ + "particleType" + suffix_);
    iEvent.put(v_processType , prefix_ + "processType"  + suffix_);
    iEvent.put(v_detUnitId   , prefix_ + "detUnitId"    + suffix_);
    iEvent.put(v_trkId       , prefix_ + "trkId"        + suffix_);
    iEvent.put(v_evtId       , prefix_ + "evtId"        + suffix_);
    iEvent.put(v_size        , prefix_ + "size"         + suffix_);
}
