import FWCore.ParameterSet.Config as cms

ntuplePixelDigis = cms.EDProducer('NTuplePixelDigis',
    inputTag = cms.InputTag('simSiPixelDigis'),
    inputTagTP = cms.InputTag('mix', 'MergedTrackTruth'),
    simHitCollections = cms.PSet(
        pixel = cms.VInputTag(
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelBarrelLowTof'),
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelBarrelHighTof'),
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelEndcapLowTof'),
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelEndcapHighTof'),
        ),
    ),
    prefix = cms.string('pixelDigis@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleTTClusters = cms.EDProducer('NTupleTTClusters',
    inputTag = cms.InputTag('TTClustersFromPixelDigis', 'ClusterInclusive'),
    inputTagMC = cms.InputTag('TTClusterAssociatorFromPixelDigis', 'ClusterInclusive'),
    inputTagDigi = ntuplePixelDigis.inputTag,
    inputTagTP = ntuplePixelDigis.inputTagTP,
    prefix = cms.string('TTClusters@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleTTStubs = cms.EDProducer('NTupleTTStubs',
    inputTag = cms.InputTag('TTStubsFromPixelDigis', 'StubAccepted'),
    inputTagMC = cms.InputTag('TTStubAssociatorFromPixelDigis', 'StubAccepted'),
    inputTagClus = ntupleTTClusters.inputTag,
    inputTagDigi = ntuplePixelDigis.inputTag,
    inputTagTP = ntuplePixelDigis.inputTagTP,
    prefix = cms.string('TTStubs@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleTTTracks = cms.EDProducer('NTupleTTTracks',
    inputTag = cms.InputTag('TTTracksFromPixelDigis', 'Level1TTTracks'),
    inputTagMC = cms.InputTag('TTTrackAssociatorFromPixelDigis', 'Level1TTTracks'),
    inputTagStub = ntupleTTStubs.inputTag,
    nparameters = cms.int32(4),
    prefix = cms.string('TrackletTTTracks@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleTTClustersForTTI = ntupleTTClusters.clone(
    inputTag = cms.InputTag('TTStubsFromPixelDigis', 'ClusterAccepted'),
    inputTagMC = cms.InputTag('TTClusterAssociatorFromPixelDigis', 'ClusterAccepted'),
    inputTagDigi = cms.InputTag(''),
)

ntupleTTStubsForTTI = ntupleTTStubs.clone(
    inputTagClus = ntupleTTClustersForTTI.inputTag,
    inputTagDigi = cms.InputTag(''),
)

ntupleL1TrackTrigger = cms.Sequence(ntuplePixelDigis * ntupleTTClusters * ntupleTTStubs * ntupleTTTracks)
ntupleL1TrackTrigger_TTI = cms.Sequence(ntupleTTClustersForTTI * ntupleTTStubsForTTI * ntupleTTTracks)

