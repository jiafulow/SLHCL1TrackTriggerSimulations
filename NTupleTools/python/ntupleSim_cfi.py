import FWCore.ParameterSet.Config as cms

ntupleSimTracks = cms.EDProducer('NTupleSimTracks',
    inputTag = cms.InputTag('g4SimHits'),
    prefix = cms.string('simTracks@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleSimVertices = cms.EDProducer('NTupleSimVertices',
    inputTag = cms.InputTag('g4SimHits'),
    prefix = cms.string('simVertices@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

ntupleSimHits = cms.EDProducer('NTupleSimHits',
    inputTag = cms.InputTag('g4SimHits'),
    simHitCollections = cms.PSet(
        pixel = cms.VInputTag(
            cms.InputTag('g4SimHits','TrackerHitsPixelBarrelLowTof'),
            cms.InputTag('g4SimHits','TrackerHitsPixelBarrelHighTof'),
            cms.InputTag('g4SimHits','TrackerHitsPixelEndcapLowTof'),
            cms.InputTag('g4SimHits','TrackerHitsPixelEndcapHighTof'),
        ),
    ),
    prefix = cms.string('simHits@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

#ntupleSim = cms.Sequence(ntupleSimTracks * ntupleSimVertices * ntupleSimHits)
ntupleSim = cms.Sequence(ntupleSimTracks * ntupleSimVertices)

