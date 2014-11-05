#!/bin/bash

# Pass in name and status
function die { echo $1: status $2 ;  exit $2; }

[ -e test_ntuple.root ] || die 'Failure getting ntuple root file' $?
[ -e test_ntuple.txt ] || echo "test_ntuple.root" > test_ntuple.txt
[ -e trigger_sector_map.csv ] || ln -s ${CMSSW_BASE}/src/SLHCL1TrackTriggerSimulations/AMSimulation/data/trigger_sector_map.csv
PYTHONTEST=${CMSSW_BASE}/src/SLHCL1TrackTriggerSimulations/AMSimulation/python/test

#(amsim --help) || die 'Failure getting help message' $?

(amsim -C -i test_ntuple.txt -o stubs.root -n 100 --no-filter --timing) || die 'Failure during stub cleaning' $?
(python ${PYTHONTEST}/testStubCleaning.py ${LOCAL_TOP_DIR}/stubs.root) || die 'Failure using tesStubCleaning.py' $?

(amsim -G -i stubs.root -o patternBank.root -L trigger_sector_map.csv -n 100 --bank_requireTriggerTower 0 --bank_nDCBits 0 --timing) || die 'Failure during pattern bank generation' $?
(python ${PYTHONTEST}/testPatternBankGeneration.py ${LOCAL_TOP_DIR}/patternBank.root) || die 'Failure using patternBankGeneration.py' $?

(amsim -R -i test_ntuple.txt -o roads.root -B patternBank.root -n 100 --bank_nDCBits 0 --timing) || die 'Failure during pattern recognition' $?
(python ${PYTHONTEST}/testPatternRecognition.py ${LOCAL_TOP_DIR}/roads.root) || die 'Failure using testPatternRecognition.py' $?

(amsim -F -i roads.root -o tracks.root -n 100 --timing) || die 'Failure during track fitting' $?
(python ${PYTHONTEST}/testTrackFitting.py ${LOCAL_TOP_DIR}/tracks.root) || die 'Failure using testTrackFitting.py' $?

(amsim -W -i test_ntuple.txt -o results.root --roads roads.root --tracks tracks.root -n 100 --timing) || die 'Failure during ntuple writing' $?
(python ${PYTHONTEST}/testWriting.py ${LOCAL_TOP_DIR}/results.root) || die 'Failure using testWriting.py' $?

(amconvert -B -i patternBank.root -o patternBank.txt -n 100 --format x) || die 'Failure during bank conversion' $?
CHARCOUNT=`wc -m < "${LOCAL_TOP_DIR}/patternBank.txt"`
[ "${CHARCOUNT}" -eq 4311 ] || die 'Failure checking the character counts' $?
