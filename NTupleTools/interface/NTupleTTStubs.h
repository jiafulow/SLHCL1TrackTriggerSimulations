#ifndef NTupleTools_NTupleTTStubs_h_
#define NTupleTools_NTupleTTStubs_h_

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleCommon.h"
#include "DataFormats/L1TrackTrigger/interface/TTStub.h"
#include "DataFormats/L1TrackTrigger/interface/TTTypes.h"

#include "Geometry/Records/interface/StackedTrackerGeometryRecord.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/StackedTrackerGeometry.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"


class NTupleTTStubs : public edm::EDProducer {
  public:
    explicit NTupleTTStubs(const edm::ParameterSet&);

  private:
    //virtual void beginJob();
    virtual void produce(edm::Event&, const edm::EventSetup&);
    //virtual void endJob();

    virtual void beginRun(const edm::Run&, const edm::EventSetup&);
    //virtual void endRun(const edm::Run&, const edm::EventSetup&);

    // For event setup
    const TrackerGeometry * theGeometry;
    const StackedTrackerGeometry * theStackedGeometry;
    const MagneticField* theMagneticField;

    const edm::InputTag inputTag_, inputTagMC_, inputTagClus_, inputTagDigi_, inputTagTP_;
    const std::string   prefix_, suffix_;

    StringCutObjectSelector<TTStub<Ref_PixelDigi_> > selector_;
    const unsigned maxN_;
};

#endif
