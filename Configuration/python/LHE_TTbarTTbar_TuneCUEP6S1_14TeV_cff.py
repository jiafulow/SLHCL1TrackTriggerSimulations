import FWCore.ParameterSet.Config as cms

source = cms.Source("LHESource",
    fileNames = cms.untracked.vstring('file:/afs/cern.ch/user/j/jiafulow/public/fromSeb/LHE/4tops_SM_10000_events.lhe'),
)

#from Configuration.Generator.Pythia6CUEP6S1Settings_cfi import *
from SLHCL1TrackTriggerSimulations.Configuration.Pythia6CUEP6S1Settings_cfi import *
from GeneratorInterface.ExternalDecays.TauolaSettings_cff import *

generator = cms.EDFilter("Pythia6HadronizerFilter",
    pythiaHepMCVerbosity = cms.untracked.bool(False),
    maxEventsToPrint = cms.untracked.int32(0),
    pythiaPylistVerbosity = cms.untracked.int32(1),
    comEnergy = cms.double(14000.0),
    PythiaParameters = cms.PSet(
        pythia6CUEP6S1SettingsBlock,
        processParameters = cms.vstring(
            'MSEL=0          ! User defined processes',
            'MSTJ(1)=1       ! Fragmentation/hadronization on or off',
            'MSTP(61)=1      ! Parton showering on or off',
            'PMAS(5,1) = 4.4    ! b quark mass',
            'PMAS(6,1) = 172.5  ! top quark mass',
        ),
        # This is a vector of ParameterSet names to be read, in this order
        parameterSets = cms.vstring(
            'pythia6CUEP6S1Settings',
            'processParameters',
        ),
    ),
    jetMatching = cms.untracked.PSet(
       scheme = cms.string("Madgraph"),
       mode = cms.string("auto"),     # soup, or "inclusive" / "exclusive"
       MEMAIN_etaclmax = cms.double(5.0),
       MEMAIN_qcut = cms.double(30.0),
       MEMAIN_minjets = cms.int32(0),
       MEMAIN_maxjets = cms.int32(12),
       MEMAIN_showerkt = cms.double(0),   # use 1=yes only for pt-ordered showers !
       MEMAIN_nqmatch = cms.int32(5), #PID of the flavor until which the QCD radiation are kept in the matching procedure; 
                                      # if nqmatch=4, then all showered partons from b's are NOT taken into account
                                      # Note (JY): I'm not sure what the default is, but -1 results in a throw...
       MEMAIN_excres = cms.string(""),
       outTree_flag = cms.int32(0)        # 1=yes, write out the tree for future sanity check
    )
)

ProductionFilterSequence = cms.Sequence(generator)
