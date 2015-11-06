import FWCore.ParameterSet.Config as cms

ntupleTrackingParticles = cms.EDProducer('NTupleTrackingParticles',
    inputTag = cms.InputTag('mix', 'MergedTrackTruth'),
    prefix = cms.string('trkParts@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleTrackingVertices = cms.EDProducer('NTupleTrackingVertices',
    inputTag = cms.InputTag('mix', 'MergedTrackTruth'),
    prefix = cms.string('trkVertices@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleMixedSimHits = cms.EDProducer('NTupleMixedSimHits',
    inputTag = cms.InputTag('mix'),
    inputTagTP = cms.InputTag('mix', 'MergedTrackTruth'),
    simHitCollections = cms.PSet(
        pixel = cms.VInputTag(
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelBarrelLowTof'),
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelBarrelHighTof'),
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelEndcapLowTof'),
            cms.InputTag('mix','g4SimHitsTrackerHitsPixelEndcapHighTof'),
        ),
    ),
    prefix = cms.string('mixedSimHits@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleDigi = cms.Sequence(ntupleTrackingParticles * ntupleTrackingVertices * ntupleMixedSimHits)

