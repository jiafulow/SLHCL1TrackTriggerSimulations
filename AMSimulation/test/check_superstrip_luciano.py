#!/usr/bin/env python

from ROOT import gROOT
gROOT.LoadMacro("../interface/HelperMath.h")  # implement etaFromRhoZ in python?
gROOT.LoadMacro("superstrip_luciano.C+O")
gROOT.LoadMacro("superstrip_rational.C+O")

from ROOT import M_PI, etaFromRhoZ, superstrip_luciano, superstrip_rational
from collections import Counter

import sys
sys.path.append('../res/')
from roothelper import b_rcoord_luciano_m as rBarrel

# Luciano's hit class
class Hit(object):
    def __init__(self, i=0, x=0., z=0.):
        self.iLayer = i
        self.x = x
        self.z = z
        self.phi = self.x / rBarrel[i]
        self.eta = etaFromRhoZ(rBarrel[i], self.z)

def make_pattern(hits):
    pat = []
    for h in hits:
        ss = superstrip_luciano(h.iLayer, h.phi, h.eta, M_PI/800., 4.4) - (h.iLayer * 1600 + 800)
        #ss = superstrip_rational(h.iLayer, h.phi, h.eta, 1.0)
        pat.append(ss)
        #print ss, h.phi, h.eta
    return tuple(pat)


# Open file
hitsfile = "hits2.txt"
prefix = ""
seqfile = prefix + "patternBankSequential.txt"
outfile = prefix + "patternBank.txt"

hits = []
unsorted_bank = []
sorted_bank = []
with open(hitsfile) as f:
    bank = Counter()

    nStep = 10000
    curSize, prevSize = 0, 0
    for i_line, line in enumerate(f):
        tokens = line.split(",")
        tokens = [float(t.strip()) for t in tokens]

        hits = []
        hit = Hit(0, tokens[1] , tokens[2] ); hits.append(hit)
        hit = Hit(1, tokens[3] , tokens[4] ); hits.append(hit)
        hit = Hit(2, tokens[5] , tokens[6] ); hits.append(hit)
        hit = Hit(3, tokens[7] , tokens[8] ); hits.append(hit)
        hit = Hit(4, tokens[9] , tokens[10]); hits.append(hit)
        hit = Hit(5, tokens[11], tokens[12]); hits.append(hit)

        pattern = make_pattern(hits)

        if pattern not in bank:
            unsorted_bank.append(pattern)

        if (i_line % nStep) == 0:
            curSize = len(bank)
            coverage = 1.-(curSize-prevSize)/float(nStep);
            print i_line, "tracks generated,", curSize, "patterns - coverage:", coverage
            prevSize = curSize

        bank[pattern] += 1

    sorted_bank = bank.most_common()

    print len(bank)
    print unsorted_bank[:10]
    print sorted_bank[:10]

with open(seqfile, 'w') as f:
    for i, pat in enumerate(unsorted_bank):
        s = "%10i%10i%8i" % (i, 0, 0)
        s += "  "
        s += ("%5i" * 6) % pat
        s += "\n"
        f.write(s)

with open(outfile, 'w') as f:
    total = 0
    for i, (pat, freq) in enumerate(sorted_bank):
        total += freq
        s = "%10i%10i%8i" % (i, total, freq)
        s += "  "
        s += ("%5i" * 6) % pat
        s += "\n"
        f.write(s)

