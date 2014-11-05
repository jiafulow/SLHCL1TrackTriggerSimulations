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

ntupleSimBeamSpot = cms.EDProducer('NTupleSimBeamSpot',
    inputTag = cms.InputTag('BeamSpotFromSim', 'BeamSpot'),
    prefix = cms.string('simBeamSpot@'),
    suffix = cms.string(''),
    cut = cms.string(''),
    maxN = cms.uint32(999999)
)

#ntuplePSimHitsBarrelLowTof = cms.EDProducer('NTuplePSimHits',
#    inputTag = cms.InputTag("g4SimHits","TrackerHitsPixelBarrelLowTof"),
#    prefix = cms.string('psimHitsBarrelLowTof@'),
#    suffix = cms.string(''),
#    cut = cms.string(''),
#    maxN = cms.uint32(999999)
#)
#
#ntuplePSimHitsEndcapLowTof = cms.EDProducer('NTuplePSimHits',
#    inputTag = cms.InputTag("g4SimHits","TrackerHitsPixelEndcapLowTof"),
#    prefix = cms.string('psimHitsEndcapLowTof@'),
#    suffix = cms.string(''),
#    cut = cms.string(''),
#    maxN = cms.uint32(999999)
#)


#ntupleSim = cms.Sequence(ntupleSimTracks * ntupleSimVertices * ntupleSimBeamSpot)
ntupleSim = cms.Sequence(ntupleSimTracks * ntupleSimVertices)

