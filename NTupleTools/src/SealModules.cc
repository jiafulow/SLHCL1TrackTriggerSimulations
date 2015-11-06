#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleBeamSpot.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleEventInfo.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleGenParticles.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleGenParticlesExtra.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleGenJets.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleGenMET.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleGenEventInfo.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleSimTracks.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleSimVertices.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleSimHits.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTrackingParticles.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTrackingVertices.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleMixedSimHits.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTuplePixelDigis.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTTClusters.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTTStubs.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleTTTracks.h"
#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleMaker.h"

DEFINE_FWK_MODULE(NTupleBeamSpot);
DEFINE_FWK_MODULE(NTupleEventInfo);
DEFINE_FWK_MODULE(NTupleGenParticles);
DEFINE_FWK_MODULE(NTupleGenParticlesExtra);
DEFINE_FWK_MODULE(NTupleGenJets);
DEFINE_FWK_MODULE(NTupleGenMET);
DEFINE_FWK_MODULE(NTupleGenEventInfo);
DEFINE_FWK_MODULE(NTupleSimTracks);
DEFINE_FWK_MODULE(NTupleSimVertices);
DEFINE_FWK_MODULE(NTupleSimHits);
DEFINE_FWK_MODULE(NTupleTrackingParticles);
DEFINE_FWK_MODULE(NTupleTrackingVertices);
DEFINE_FWK_MODULE(NTupleMixedSimHits);
DEFINE_FWK_MODULE(NTuplePixelDigis);
DEFINE_FWK_MODULE(NTupleTTClusters);
DEFINE_FWK_MODULE(NTupleTTStubs);
DEFINE_FWK_MODULE(NTupleTTTracks);
DEFINE_FWK_MODULE(NTupleMaker);

