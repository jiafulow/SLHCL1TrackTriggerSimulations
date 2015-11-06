#!/usr/bin/env python

from rootdrawing import *
from parser import *
from math import sinh
import random

col  = TColor.GetColor("#1f78b4")  # mu0
fcol = TColor.GetColor("#a6cee3")  # mu0


# ______________________________________________________________________________
parnames = ["#phi", "cot #theta", "z_{0} [cm]", "1/p_{T} [1/GeV]"]
nvariables = 12
nparameters = 4

parrmses = [0.000453, 0.002403, 0.083406, 0.001283]


# ______________________________________________________________________________
def drawer_book(options):
    histos = {}

    for i in xrange(nparameters):
        hname = "errpar%i" % i
        histos[hname] = TH1F(hname, "; #Delta%s" % parnames[i], 1000, -0.1, 0.1)
        histos[hname].rms = parrmses[i]

    # Change binning
    histos["errpar0"].SetBins(200, -0.005, 0.005)
    histos["errpar1"].SetBins(200, -0.02, 0.02)
    histos["errpar2"].SetBins(200, -0.5, 0.5)
    histos["errpar3"].SetBins(200, -0.02, 0.02)

    # Clone
    for i in xrange(nparameters):
        hname = "errpar%i" % i
        hname1 = hname + "_random"
        histos[hname1] = histos[hname].Clone(hname1)
        histos[hname1].rms = parrmses[i]

    # Style
    for hname, h in histos.iteritems():
        h.SetLineWidth(2); h.SetMarkerSize(0)
        h.SetLineColor(col); h.SetFillColor(fcol)
        h.SetFillStyle(0)
    donotdelete.append(histos)
    return histos

def drawer_project(tree, histos, options):
    tree.SetBranchStatus("*", 0)
    tree.SetBranchStatus("trkParts_pt"     , 1)
    tree.SetBranchStatus("trkParts_eta"    , 1)
    tree.SetBranchStatus("trkParts_phi"    , 1)
    #tree.SetBranchStatus("trkParts_vx"     , 1)
    #tree.SetBranchStatus("trkParts_vy"     , 1)
    tree.SetBranchStatus("trkParts_vz"     , 1)
    tree.SetBranchStatus("trkParts_charge" , 1)
    tree.SetBranchStatus("trkParts_primary", 1)
    tree.SetBranchStatus("trkParts_signal" , 1)
    tree.SetBranchStatus("trkParts_pdgId"  , 1)
    #tree.SetBranchStatus("AMTTRoads_patternRef" , 1)
    #tree.SetBranchStatus("AMTTRoads_stubRefs"   , 1)
    tree.SetBranchStatus("AMTTTracks_invPt"     , 1)
    tree.SetBranchStatus("AMTTTracks_phi0"      , 1)
    tree.SetBranchStatus("AMTTTracks_cottheta"  , 1)
    tree.SetBranchStatus("AMTTTracks_z0"        , 1)
    tree.SetBranchStatus("AMTTTracks_chi2"      , 1)
    tree.SetBranchStatus("AMTTTracks_ndof"      , 1)
    tree.SetBranchStatus("AMTTTracks_synTpId"   , 1)
    tree.SetBranchStatus("AMTTTracks_patternRef", 1)
    #tree.SetBranchStatus("AMTTTracks_stubRefs"  , 1)

    def absDiff(lhs, rhs):
        return abs(lhs - rhs)

    def squaredNormDiff(lhs, rhs, scale):
        return ((lhs - rhs)/scale)**2

    trkparts_allevts = []
    tracks_allevts = []

    # Loop over events
    for ievt, evt in enumerate(tree):
        if (ievt == options.nentries):  break

        if (ievt % 100 == 0):  print "Processing event: %i" % ievt

        nparts_all = evt.trkParts_primary.size()
        trkparts = {}

        for ipart in xrange(nparts_all):
            if options.pu == 0:  # single-track events
                if ipart > 0:
                    break

            charge  = evt.trkParts_charge [ipart]
            primary = evt.trkParts_primary[ipart]
            signal  = evt.trkParts_signal [ipart]

            if not (charge!=0 and primary):
                continue

            if options.signal and not signal:
                continue

            pt      = evt.trkParts_pt     [ipart]
            eta     = evt.trkParts_eta    [ipart]
            phi     = evt.trkParts_phi    [ipart]
            #vx      = evt.trkParts_vx     [ipart]
            #vy      = evt.trkParts_vy     [ipart]
            vz      = evt.trkParts_vz     [ipart]
            #pdgId   = evt.trkParts_pdgId  [ipart]

            trkparts[ipart] = (phi, sinh(eta), vz, float(charge)/pt)
            if options.verbose:  print ievt, "part ", ipart, trkparts[ipart]

        ntracks_all = evt.AMTTTracks_patternRef.size()
        tracks = {}

        for itrack in xrange(ntracks_all):
            patternRef = evt.AMTTTracks_patternRef[itrack]
            if not (patternRef < options.npatterns):
                continue

            chi2     = evt.AMTTTracks_chi2[itrack]
            ndof     = evt.AMTTTracks_ndof[itrack]
            chi2Red  = chi2/ndof

            if not (chi2Red < options.maxChi2):
                continue

            phi0       = evt.AMTTTracks_phi0      [itrack]
            cottheta   = evt.AMTTTracks_cottheta  [itrack]
            z0         = evt.AMTTTracks_z0        [itrack]
            invPt      = evt.AMTTTracks_invPt     [itrack]

            tracks[itrack] = (phi0, cottheta, z0, invPt)

        # Sanity check
        for ipart, part in trkparts.iteritems():
            minMatch, minMatch_i = 999999., 0
            for itrack, track in tracks.iteritems():
                match = 0.
                for i in xrange(4):
                    match += squaredNormDiff(part[i], track[i], parrmses[i])

                if minMatch > match:
                    minMatch = match
                    minMatch_i = itrack
                #print ievt, ipart, itrack, "(%i)" % evt.AMTTTracks_synTpId[itrack], match, minMatch, minMatch_i

            if minMatch != 999999. and minMatch < 5:
                if ipart != evt.AMTTTracks_synTpId[minMatch_i]:
                    print "WARNING: not match: ", ipart, evt.AMTTTracks_synTpId[minMatch_i]


        # Fill histograms
        for ipart, part in trkparts.iteritems():
            trkparts_allevts.append(part)

        for itrack, track in tracks.iteritems():
            tracks_allevts.append(track)

        for ipart, part in trkparts.iteritems():
            for itrack, track in tracks.iteritems():
                #if not (ipart == evt.AMTTTracks_synTpId[itrack]):
                #    continue

                for i in xrange(4):
                    accept = ((absDiff(part[0], track[0]) < parrmses[0] * 5) or i == 0) and \
                             ((absDiff(part[1], track[1]) < parrmses[1] * 5) or i == 1) and \
                             ((absDiff(part[2], track[2]) < parrmses[2] * 5) or i == 2) and \
                             ((absDiff(part[3], track[3]) < parrmses[3] * 5) or i == 3)
                    if accept:
                        histos["errpar%i" % i].Fill(track[i] - part[i])

    options.nentries = ievt
    assert(len(trkparts_allevts) == options.nentries)

    # Fill histograms
    for ievt in xrange(options.nentries):
        if (ievt % 100 == 0):  print "Processing event: %i" % ievt

        part = trkparts_allevts[ievt]

        for j in xrange(1000):
            track = random.choice(tracks_allevts)

            for i in xrange(4):
                accept = ((absDiff(part[0], track[0]) < parrmses[0] * 5) or i == 0) and \
                         ((absDiff(part[1], track[1]) < parrmses[1] * 5) or i == 1) and \
                         ((absDiff(part[2], track[2]) < parrmses[2] * 5) or i == 2) and \
                         ((absDiff(part[3], track[3]) < parrmses[3] * 5) or i == 3)
                if accept:
                    histos["errpar%i_random" % i].Fill(track[i] - part[i])

    tree.SetBranchStatus("*", 1)
    return

def drawer_draw(histos, options):
    def displayGaus(h, ignorebins=0, scalebox=(1.,1.)):
        # Display gaussian fit parameters
        fitxmin, fitxmax = h.GetBinCenter(ignorebins+1), h.GetBinCenter(h.GetNbinsX()-1-ignorebins+1)
        r = h.Fit("gaus", "INS", "", fitxmin, fitxmax)
        p1, e1, p2, e2 = r.Parameter(1), r.ParError(1), r.Parameter(2), r.ParError(2)

        gPad.Modified(); gPad.Update()
        ps = h.FindObject("stats")
        ps.SetName("mystats")

        n = 2
        newX1NDC = ps.GetX2NDC() - (ps.GetX2NDC() - ps.GetX1NDC()) * scalebox[0]
        newY1NDC = ps.GetY2NDC() - ((ps.GetY2NDC() - ps.GetY1NDC()) / 5 * (5 + n)) * scalebox[1]
        ps.SetX1NDC(newX1NDC)
        ps.SetY1NDC(newY1NDC)

        ps.AddText("Gaus #mu = %6.4g" % (p1))
        ps.AddText("Gaus #sigma = %6.4g" % (p2))
        #ps.AddText("Gaus #mu = %6.4g #pm %6.4g" % (p1, e1))
        #ps.AddText("Gaus #sigma = %6.4g #pm %6.4g" % (p2, e2))
        h.stats = [p1, e1, p2, e2]

        h.SetStats(0)
        #gPad.Modified(); gPad.Update()
        ps.Draw()


    for hname, h in histos.iteritems():
        ymax = h.GetMaximum() * 1.4
        h.SetMinimum(0); h.SetMaximum(ymax)

        h.Draw("hist")
        displayGaus(h, ignorebins=10)

        tline.DrawLine(h.rms * -5, 0, h.rms * -5, ymax)
        tline.DrawLine(h.rms *  5, 0, h.rms *  5, ymax)

        CMS_label()
        save(options.outdir, "%s_%s" % (hname, options.ss), dot_root=True)
    return

def drawer_sitrep(histos, options):
    print "--- SITREP ---------------------------------------------------------"


# ______________________________________________________________________________
# Main function
def main(options):

    # Init
    drawerInit = DrawerInit()
    tchain = TChain("ntupler/tree", "")
    tchain.AddFileInfoList(options.tfilecoll.GetList())

    # Process
    histos = drawer_book(options)
    drawer_project(tchain, histos, options)
    drawer_draw(histos, options)
    drawer_sitrep(histos, options)


# ______________________________________________________________________________
if __name__ == '__main__':

    # Setup argument parser
    parser = argparse.ArgumentParser()

    # Add default arguments
    add_drawer_arguments(parser)

    # Add more arguments
    parser.add_argument("ss", help="short name of superstrip definition (e.g. ss256)")
    parser.add_argument("npatterns", type=int, help="number of patterns to reach the desired coverage")
    parser.add_argument("--coverage", type=float, default=0.95, help="desired coverage (default: %(default)s)")
    #parser.add_argument("--minPt", type=float, default=2, help="min pT for gen particle (default: %(default)s)")
    parser.add_argument("--maxChi2", type=float, default=5, help="max reduced chi-squared (default: %(default)s)")

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)

    # Call the main function
    main(options)
