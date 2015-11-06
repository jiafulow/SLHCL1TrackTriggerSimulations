#ifndef NTupleTools_NTupleGenParticlesExtra_h_
#define NTupleTools_NTupleGenParticlesExtra_h_

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleCommon.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"


namespace reco { class GenParticle; }

class NTupleGenParticlesExtra : public edm::EDProducer {
  public:
    explicit NTupleGenParticlesExtra(const edm::ParameterSet&);

  private:
    //virtual void beginJob();
    virtual void produce(edm::Event&, const edm::EventSetup&);
    //virtual void endJob();

    virtual void beginRun(const edm::Run&, const edm::EventSetup&);
    //virtual void endRun(const edm::Run&, const edm::EventSetup&);

    // For event setup
    const MagneticField* theMagneticField;

    const edm::InputTag inputTag_;
    const edm::InputTag inputTagBeamSpot_;
    const std::string   prefix_, suffix_;

    StringCutObjectSelector<reco::GenParticle> selector_;
    const unsigned maxN_;
};

#endif
