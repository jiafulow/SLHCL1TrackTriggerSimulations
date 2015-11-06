#ifndef NTupleTools_NTupleTTTracks_h_
#define NTupleTools_NTupleTTTracks_h_

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleCommon.h"
#include "DataFormats/L1TrackTrigger/interface/TTTrack.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"


class NTupleTTTracks : public edm::EDProducer {
  public:
    explicit NTupleTTTracks(const edm::ParameterSet&);

  private:
    //virtual void beginJob();
    virtual void produce(edm::Event&, const edm::EventSetup&);
    //virtual void endJob();

    const edm::InputTag inputTag_, inputTagMC_, inputTagStub_;
    const int nparameters_;
    const std::string   prefix_, suffix_;

    StringCutObjectSelector<TTTrack<Ref_PixelDigi_> > selector_;
    const unsigned maxN_;
};

#endif
