import FWCore.ParameterSet.Config as cms
import os

process = cms.Process("GeomInfo")

process.load('Configuration.Geometry.GeometryExtended2023TTIReco_cff')
process.load('Configuration.Geometry.GeometryExtended2023TTI_cff')
process.load('Geometry.TrackerGeometryBuilder.StackedTrackerGeometry_cfi')

process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:upgradePLS3', '')

process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.geominfo = cms.EDAnalyzer("AnalyzerPatternGeomInfo",
    #bank = cms.string('patternBank.root'),
    bank = cms.string('patternBank_ss32_22M.root'),
    verbosity = cms.int32(0),
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string('patternGeomInfo_Extended2023TTI.root'),
)

process.p1 = cms.Path(process.geominfo)
process.schedule = cms.Schedule(process.p1)
