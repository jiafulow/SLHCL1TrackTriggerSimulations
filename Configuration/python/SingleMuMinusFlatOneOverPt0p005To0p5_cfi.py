import FWCore.ParameterSet.Config as cms

# reference: https://github.com/cms-sw/genproductions/blob/master/python/EightTeV/SingleMuMinusFlatPt0p2To100_cff.py
generator = cms.EDProducer("FlatRandomOneOverPtGunProducer",
    PGunParameters = cms.PSet(
        MaxOneOverPt = cms.double(0.5),
        MinOneOverPt = cms.double(0.005),
        PartID = cms.vint32(13),
        MaxEta = cms.double(2.5),
        MaxPhi = cms.double(3.14159265359),
        MinEta = cms.double(-2.5),
        MinPhi = cms.double(-3.14159265359),
        #XFlatSpread = cms.double(1.5),  ## in mm
        #YFlatSpread = cms.double(1.5),  ## in mm
        #ZFlatSpread = cms.double(150.), ## in mm
        #ReallyFlat = cms.bool(True),
    ),
    Verbosity = cms.untracked.int32(0),
    psethack = cms.string('single mu- 1/pt 0.005 to 0.5'),
    AddAntiParticle = cms.bool(False),
    firstRun = cms.untracked.uint32(1)
)
