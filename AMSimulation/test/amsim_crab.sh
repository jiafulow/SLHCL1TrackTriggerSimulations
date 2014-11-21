#!/bin/bash

########################################
# Run amsim with CRAB
#
# copied from HiggsAnalysis/CombinedLimit/test/combine_crab.sh
#
########################################

i="$1"
if [ "$i" == "help" ]; then
  echo "usage: amsim_crab.sh <job index> [<max events>]"
  exit 0;
fi
if [ "$i" = "" ]; then
  echo "Error: missing job index"
  exit 1;
fi

n="$MaxEvents" 
#if [ "$n" = "" ]; then
#  n="$2"
#fi
if [ "$n" = "" ]; then
  echo "Error: missing number of events"
  exit 2;
fi

args=("${@:2}")

# Rename
#echo "Files in current working directory (before rename): "
#ls .
if  ls input_*.txt > /dev/null 2>&1; then
    mv input_*.txt input.txt

fi
if  ls make_symlinks_*.sh > /dev/null 2>&1; then
    mv make_symlinks_*.sh make_symlinks.sh

    chmod a+x make_symlinks.sh
fi
#echo "Files in current working directory (after rename): "
#ls .


echo "Running on input source:"
cat input.txt
echo "  max events: $n"
echo "  arguments: ${args[@]}"

echo "Make symlinks:"
cat make_symlinks.sh
./make_symlinks.sh

echo "Setup fas library:"
wget -q --no-check-certificate https://raw.githubusercontent.com/jiafulow/SLHCL1TrackTriggerSimulations/fixes/fas.xml
cp fas.xml $CMSSW_BASE/config/toolbox/$SCRAM_ARCH/tools/selected/
scram setup fas

echo "Running amsim:"
./amsim.exe -R -i input.txt -o roads.root -n "$n" "${args[@]}" --no-color >& amsimR.log
./amsim.exe -F -i roads.root -o tracks.root -n "$n" "${args[@]}" --no-color >& amsimF.log
./amsim.exe -W -i input.txt -o results.root --roads roads.root --tracks tracks.root -n "$n" "${args[@]}" --no-color >& amsimW.log
cat amsimR.log amsimF.log amsimW.log

echo "<FrameworkJobReport>
	<ExitCode Value=\"0\"/>
	<TimingService/>
	<PerformanceReport>
		<CPU/>
		<Memory/>
	</PerformanceReport>
</FrameworkJobReport>" > ../crab_fjr_$NJob.xml

