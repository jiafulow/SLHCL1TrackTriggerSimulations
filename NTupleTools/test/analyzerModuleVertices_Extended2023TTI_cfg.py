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

process.a1 = cms.EDAnalyzer("AnalyzerModuleVertices",
    csv = cms.string('module_vertices.csv'),
    verbosity = cms.int32(0),
)

process.p1 = cms.Path(process.a1)
process.schedule = cms.Schedule(process.p1)

