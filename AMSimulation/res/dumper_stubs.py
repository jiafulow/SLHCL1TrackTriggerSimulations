from ROOT import TFile
from itertools import izip
import json
from roothelper import convert_key_to_int
from roothelper import b_rcoord_luciano_m as rBarrel


infilename = "/uscms_data/d2/jiafu/L1TrackTrigger/CRAB_amsim/stubs_barrel_400M.3.root"
outfilename = "../test/hits2.txt"
treename = "ntupler/tree"

#nentries = 10
#nentries = 100000
nentries = 25000000


tfile = TFile.Open(infilename)
ttree = tfile.Get(treename)

ttree.SetBranchStatus("*", 0)
ttree.SetBranchStatus("TTStubs_r"    , 1)
ttree.SetBranchStatus("TTStubs_phi"  , 1)
ttree.SetBranchStatus("TTStubs_z"    , 1)
ttree.SetBranchStatus("TTStubs_modId", 1)

ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)

writeme = []

j_ievt = 0
for i_ievt, ievt in enumerate(ttree):
    #if (i_ievt == nentries):  break
    if (j_ievt == nentries):  break

    # Exactly 6 stubs
    if len(ievt.TTStubs_r) != 6:
        continue

    # One stub per layer per tower
    layers = []    
    for modId in ievt.TTStubs_modId:
        if modId in ttmap[27]:
            layers.append(modId/10000)
    if layers != [5,6,7,8,9,10]:
        continue

    j_ievt += 1

    # Stub coordinates
    stubs = []
    for r, phi, z in izip(ievt.TTStubs_r, ievt.TTStubs_phi, ievt.TTStubs_z):
        stubs.append((phi, z))

    # Write
    s = "%10i, " % j_ievt
    for iL, stub in enumerate(stubs):
        s += ("%.6f, %.6f, " % (stub[0] * rBarrel[iL], stub[1] * 0.01))
    s = s[:-2]
    writeme.append(s)
    #print s

with open(outfilename, 'w') as f:
    f.write('\n'.join(writeme))
    f.write('\n')

