import FWCore.ParameterSet.Config as cms

# reference: https://github.com/cms-sw/genproductions/blob/master/python/EightTeV/SingleMuMinusFlatPt0p2To100_cff.py
generator = cms.EDProducer("FlatRandomOneOverPtGunProducer",
    PGunParameters = cms.PSet(
        MaxOneOverPt = cms.double(5),
        MinOneOverPt = cms.double(0.005),
        PartID = cms.vint32(13),
        MaxEta = cms.double(0.9),
        MaxPhi = cms.double(1.8),
        MinEta = cms.double(-0.2),
        MinPhi = cms.double(0.6),
        #XFlatSpread = cms.double(1.5),  ## in mm
        #YFlatSpread = cms.double(1.5),  ## in mm
        #ZFlatSpread = cms.double(150.), ## in mm
        RandomCharge = cms.bool(True),
        #ReallyFlat = cms.bool(True),
    ),
    Verbosity = cms.untracked.int32(0),
    psethack = cms.string('single muon+/- 1/pt 0.005 to 5 tower 27'),
    AddAntiParticle = cms.bool(False),
    firstRun = cms.untracked.uint32(1)
)
