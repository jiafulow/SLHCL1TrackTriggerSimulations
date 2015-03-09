from math import floor
import json

# ______________________________________________________________________________
# Constants

b_nlads = [16, 24, 34, 48, 62, 76]
b_nmods = [63, 55, 54, 24, 24, 24]

e_offset = 5
e_nrings = [15 + e_offset]
e_nmods  = [20, 24, 28, 28, 32, 36, 36, 40, 40, 52, 56, 64, 68, 76, 80]

b_rcoord_luciano_m = [0.2243, 0.3557, 0.5059, 0.6833, 0.9, 1.077]
b_rcoord_luciano = [x*1e2 for x in b_rcoord_luciano_m]  # in cm
b_rcoord_tklayout_mm = [230.000, 357.368, 508.000, 686.000, 887.901, 1080.000]
b_rcoord_tklayout = [x*1e-1 for x in b_rcoord_tklayout_mm]  # in cm
b_rcoord_cmssw = [22.5913, 35.4772, 50.5402, 68.3101, 88.5002, 107.71]

e_zcoord_tklayout_mm = [1349.445, 1597.452, 1891.039, 2238.583, 2650.00]
e_zcoord_tklayout = [x*1e-1 for x in e_zcoord_tklayout_mm]  # in cm
e_zcoord_cmssw = [131.680667, 156.481667, 185.840667, 220.594667, 261.736667]

tt_netas = 6
tt_nphis = 8

# ______________________________________________________________________________
# Binning

class EtaBinning:
    def __init__(self, xtitle, nbinsx, xlow, xup):
        self.xtitle = xtitle
        self.nbinsx, self.xlow, self.xup = nbinsx, xlow, xup
        self.binwidth = float(self.xup - self.xlow) / self.nbinsx

    def findBin(self, x):
        if x <  self.xlow: x = self.xlow
        if x >= self.xup : x = self.xup - 1e-6
        x = float(x - self.xlow) / self.binwidth
        return int(x)

    def getBinCenter(self, b):
        if b < 0           : b = 0
        if b >= self.nbinsx: b = self.nbinsx - 1
        b = self.xlow + (0.5+b) * self.binwidth
        return b

# ______________________________________________________________________________
# Encoding

def halfStripRound(x):
    p = 10.
    return int(floor((x*2)*p + 0.5)/p)

def decodeLayer(moduleId):
    return moduleId / 10000

def decodeLadder(moduleId):
    return (moduleId / 100) % 100

def decodeModule(moduleId):
    return moduleId % 100

def isPSModule(moduleId):
    lay = decodeLayer(moduleId)
    if 5 <= lay <= 7:
        return True
    lad = decodeLadder(moduleId)
    if 11 <= lay <= 22 and lad <= 8:
        return True
    return False

def isBarrelModule(moduleId):
    lay = decodeLayer(moduleId)
    if 5 <= lay <= 10:
        return True
    return False


# ______________________________________________________________________________
# Useful functions

convert_key_to_int = lambda pairs: dict([(int(k),v) for (k,v) in pairs])

def get_reverse_map(direct_map):
    reverse_map = {}
    for i in xrange(6*8):
        for m in direct_map[i]:
            reverse_map.setdefault(m, []).append(i)
    return reverse_map

def parse_fileinfolist(txtfile, fast=False):
    infiles = []
    with open(txtfile) as f:
        for line in f:
            infiles.append(line.strip())
    if fast:
        infiles = infiles[:2]
    return infiles

