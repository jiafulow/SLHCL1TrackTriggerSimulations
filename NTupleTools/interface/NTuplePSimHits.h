#ifndef NTupleTools_NTuplePSimHits_h_
#define NTupleTools_NTuplePSimHits_h_

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleCommon.h"


class PSimHit;

class NTuplePSimHits : public edm::EDProducer {
  public:
    explicit NTuplePSimHits(const edm::ParameterSet&);

  private:
    //virtual void beginJob();
    virtual void produce(edm::Event&, const edm::EventSetup&);
    //virtual void endJob();

    const edm::InputTag inputTag_;
    const std::string   prefix_, suffix_;

    StringCutObjectSelector<PSimHit> selector_;
    const unsigned maxN_;
};

#endif
