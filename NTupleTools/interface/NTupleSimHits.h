#ifndef NTupleTools_NTupleSimHits_h_
#define NTupleTools_NTupleSimHits_h_

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleCommon.h"

#include "Geometry/Records/interface/StackedTrackerGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"


class PSimHit;

class NTupleSimHits : public edm::EDProducer {
  public:
    explicit NTupleSimHits(const edm::ParameterSet&);

  private:
    //virtual void beginJob();
    virtual void produce(edm::Event&, const edm::EventSetup&);
    //virtual void endJob();

    virtual void beginRun(const edm::Run&, const edm::EventSetup&);
    //virtual void endRun(const edm::Run&, const edm::EventSetup&);

    // For event setup
    const TrackerGeometry * theGeometry;

    const edm::InputTag         inputTag_;
    edm::ParameterSet           simHitCollectionConfig_;
    std::vector<edm::InputTag>  simHitCollections_;
    const std::string           prefix_, suffix_;

    StringCutObjectSelector<PSimHit> selector_;
    const unsigned maxN_;
};

#endif
