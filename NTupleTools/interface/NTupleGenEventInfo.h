#ifndef NTupleTools_NTupleGenEventInfo_h_
#define NTupleTools_NTupleGenEventInfo_h_

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleCommon.h"


class NTupleGenEventInfo : public edm::EDProducer {
  public:
    explicit NTupleGenEventInfo(const edm::ParameterSet&);

  private:
    //virtual void beginJob();
    virtual void produce(edm::Event&, const edm::EventSetup&);
    //virtual void endJob();

    const edm::InputTag genEventInfoTag_;
    const edm::InputTag pileupInfoTag_;
    const edm::InputTag pileupWeightTag_;
    const edm::InputTag pdfWeightTag_;
    const edm::InputTag randomSeedTag_;
    const std::string   prefix_, suffix_;
};

#endif

