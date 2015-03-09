import FWCore.ParameterSet.Config as cms


#fname = "input_MinBias_TuneZ2star_TTI2023Upg14.txt"
#fname = "input_MinBias_TuneZ2star_private_20140926.txt"
fname = "input_MinBias_TuneCUEP6S1_private_20140926.txt"

import os
dirname = os.environ["CMSSW_BASE"] + "/src/SLHCL1TrackTriggerSimulations/Configuration/data/"

with open(dirname + fname) as f:
    mix_input_filenames = f.read().splitlines()
    assert(len(mix_input_filenames)>0)


if __name__ == "__main__":
    print mix_input_filenames

