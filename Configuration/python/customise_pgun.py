import FWCore.ParameterSet.Config as cms

# Reduce particle gun sample size
# In CMSSW_6_2_0_SLHC12_patch1, edm output size is reduced to ~2.2 KB/evt
def cust_RAWSIMoutput(process):

    # Keep track of random number seeds
    process.load('SLHCL1TrackTriggerSimulations.NTupleTools.randomEngineSeedKeeper_cfi')
    process.pgen += process.randomEngineSeedKeeper

    # Modify event content
    process.RAWSIMoutput.outputCommands += [
        'drop *_MEtoEDMConverter_*_*',
        'drop *_randomEngineStateProducer_*_*',
        'drop *_logErrorHarvester_*_*',
        'drop *_simEcalDigis_*_*',
        'drop *_simHcalUnsuppressedDigis_*_*',
        'drop recoGenJets_*_*_*',
        'drop recoGenMETs_*_*_*',
        'keep *_mix_MergedTrackTruth_*',
        'keep *_randomEngineSeedKeeper_*_*',
    ]
    return (process)


# Run in tracker only, ignoring calorimeter and muon system
# In CMSSW_6_2_0_SLHC12_patch1, processing speed is reduced to ~0.06 s/evt
def cust_useTrackerOnly(process, intime=True, ntuple=True):

    # __________________________________________________________________________
    # Customise generation step

    # Remove jets & MET
    process.pgen = process.pgen.copyAndExclude([process.genJetMET])

    # Keep track of random number seeds
    process.load('SLHCL1TrackTriggerSimulations.NTupleTools.randomEngineSeedKeeper_cfi')
    process.pgen += process.randomEngineSeedKeeper

    # __________________________________________________________________________
    # Customise simulation step

    # Remove sensitive detectors
    for removee in ['MuonSD', 'CaloSD', 'CaloResponse', 'ECalSD', 'HCalSD', 'CaloTrkProcessing', 'HFShower', 'HFShowerLibrary', 'HFShowerPMT', 'HFShowerStraightBundle', 'HFShowerConicalBundle', 'HFGflash', 'CastorSD', 'CastorShowerLibrary', 'TotemSD', 'ZdcSD', 'ZdcShowerLibrary', 'FP420SD', 'BscSD', 'BHMSD', 'CFCSD', 'HGCSD', 'ShashlikSD', 'PltSD', 'HcalTB02SD', 'EcalTBH4BeamSD', 'HcalTB06BeamSD']:
        if hasattr(process.g4SimHits, removee):
            delattr(process.g4SimHits, removee)

    # Kill delta rays
    process.g4SimHits.StackingAction.KillDeltaRay = True

    # Modify geometry
    geoms_orig = process.XMLIdealGeometryESSource.geomXMLFiles
    geoms_modified = []
    for geom in geoms_orig:
        keep = True
        for removee in ['EcalCommonData', 'HcalCommonData', 'MuonCommonData', 'ForwardCommonData', 'EcalSimData', 'HcalSimData', 'MuonSimData', 'ForwardSimData', 'DTGeometryBuilder', 'CSCGeometryBuilder', 'RPCGeometryBuilder', 'GEMGeometryBuilder']:
            if geom.startswith('Geometry/%s/' % removee):
                keep = False
                break
        for removee in ['caloBase.xml', 'cmsCalo.xml', 'muonBase.xml', 'cmsMuon.xml', 'mgnt.xml', 'muonMB.xml', 'muonMagnet.xml', 'cavern.xml']:
            if geom.startswith('Geometry/CMSCommonData/data/%s' % removee):
                keep = False
                break
        if keep:
            geoms_modified.append(geom)
    process.XMLIdealGeometryESSource.geomXMLFiles = geoms_modified

    # __________________________________________________________________________
    # Customise digitisation step

    # Drop calorimeter and muon system
    process.pdigi_valid = process.pdigi_valid.copyAndExclude([process.doAllDigi])

    # No OOT mixing
    if intime:
        process.mix.minBunch = 0
        process.mix.maxBunch = 0

    # Drop sim hits in muon system
    process.mix.digitizers.mergedtruth.simHitCollections.muon = cms.VInputTag()

    # Reduce number of tracking particles
    process.mix.digitizers.mergedtruth.alwaysAddAncestors = cms.bool(False)
    process.mix.digitizers.mergedtruth.maximumPreviousBunchCrossing = cms.uint32(0)
    process.mix.digitizers.mergedtruth.maximumSubsequentBunchCrossing = cms.uint32(0)
    process.mix.digitizers.mergedtruth.ignoreTracksOutsideVolume = cms.bool(True)
    process.mix.digitizers.mergedtruth.select.ptMinTP = cms.double(0.2)
    #process.mix.digitizers.mergedtruth.select.minHitTP = cms.int32(0)
    process.mix.digitizers.mergedtruth.select.tipTP = process.mix.digitizers.mergedtruth.volumeRadius
    process.mix.digitizers.mergedtruth.select.lipTP = process.mix.digitizers.mergedtruth.volumeZ

    # Drop digitizers
    for removee in ['ecal', 'hcal', 'castor']:
        if hasattr(process.mix.digitizers, removee):
            delattr(process.mix.digitizers, removee)

    # Drop crossing frames in muon system
    for removee in ['MuonCSCHits', 'MuonDTHits', 'MuonRPCHits']:
        if removee in process.mix.mixObjects.mixSH.crossingFrames:
            process.mix.mixObjects.mixSH.crossingFrames.remove(removee)

    # Drop sub detectors
    for removee in ['BSCHits', 'FP420SI', 'MuonCSCHits', 'MuonDTHits', 'MuonRPCHits', 'TotemHitsRP', 'TotemHitsT1', 'TotemHitsT2Gem', 'TrackerHitsTECHighTof', 'TrackerHitsTECLowTof', 'TrackerHitsTIBHighTof', 'TrackerHitsTIBLowTof', 'TrackerHitsTIDHighTof', 'TrackerHitsTIDLowTof', 'TrackerHitsTOBHighTof', 'TrackerHitsTOBLowTof']:
        if removee in process.mix.mixObjects.mixSH.subdets:
            process.mix.mixObjects.mixSH.subdets.remove(removee)
        removeee = cms.InputTag("g4SimHits", removee)
        if removeee in process.mix.mixObjects.mixSH.input:
            process.mix.mixObjects.mixSH.input.remove(removeee)

    # Drop calo hits
    for removee in ['mixCH']:
        if hasattr(process.mix.mixObjects, removee):
            delattr(process.mix.mixObjects, removee)

    # Drop EDAliases
    for removee in ['simCastorDigis', 'simEcalUnsuppressedDigis', 'simHcalUnsuppressedDigis']:
        if hasattr(process, removee):
            delattr(process, removee)

    # __________________________________________________________________________
    # Customise L1TrackTrigger step

    # Drop tracklet method
    process.L1TrackTrigger = process.L1TrackTrigger.copyAndExclude([process.TrackTriggerTTTracks, process.TrackTriggerAssociatorTracks])

    # __________________________________________________________________________
    # Customise output steps

    if ntuple:
        # Drop RAWSIM output
        for removee in [process.genfiltersummary_step, process.endjob_step, process.RAWSIMoutput_step]:
            if removee in process.schedule:
                process.schedule.remove(removee)

        # Substitute with TFileService
        outputFileName = process.RAWSIMoutput.fileName._value
        outputFileName = outputFileName.replace(".root", "_ntuple.root")
        process.TFileService = cms.Service("TFileService",
            fileName = cms.string(outputFileName)
        )

        # Straight to ntuples
        process.load("SLHCL1TrackTriggerSimulations.NTupleTools.sequences_cff")
        process.p = cms.Path(process.ntupleSequence)
        process.schedule.append(process.p)

    return (process)

