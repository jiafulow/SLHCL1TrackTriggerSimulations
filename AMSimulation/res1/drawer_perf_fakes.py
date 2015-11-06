#!/usr/bin/env python

from rootdrawing import *
from parser import *

col  = TColor.GetColor("#1f78b4")  # mu0
fcol = TColor.GetColor("#a6cee3")  # mu0

#col  = TColor.GetColor("#e31a1c")  # nu140
#fcol = TColor.GetColor("#fb9a99")  # nu140

#col  = TColor.GetColor("#6a3d9a")  # tttt140
#fcol = TColor.GetColor("#cab2d6")  # tttt140

col1 = TColor.GetColor("#BA0000")
col2 = TColor.GetColor("#BA00BA")
col3 = TColor.GetColor("#3333BA")
fcol1 = TColor.GetColor("#E58080")
fcol2 = TColor.GetColor("#E580E5")
fcol3 = TColor.GetColor("#9999E5")


# ______________________________________________________________________________
def drawer_book():
    histos = {}

    hname = "nroads_per_event"
    nbins, xmin, xmax = 200, 0., 400.*options.xscale
    histos[hname] = TH1F(hname, "; # roads/tower/BX"                , nbins, xmin, xmax)

    hname = "ncombinations_per_event"
    nbins, xmin, xmax = 800, 0., 1600.*options.xscale
    histos[hname] = TH1F(hname, "; # combinations/tower/BX"         , nbins, xmin, xmax)

    hname = "ntracks_per_event"
    nbins, xmin, xmax = 80, 0., 80.*options.xscale
    histos[hname] = TH1F(hname, "; # tracks/tower/BX"               , nbins, xmin, xmax)

    hname = "ngoods_per_event"
    nbins, xmin, xmax = 80, 0., 80.*options.xscale
    histos[hname] = TH1F(hname, "; # good tracks/tower/BX"          , nbins, xmin, xmax)

    hname = "nduplicates_per_event"
    nbins, xmin, xmax = 80, 0., 80.*options.xscale
    histos[hname] = TH1F(hname, "; # duplicate tracks/tower/BX"     , nbins, xmin, xmax)

    hname = "nfakes_per_event"
    nbins, xmin, xmax = 80, 0., 80.*options.xscale
    histos[hname] = TH1F(hname, "; # fake tracks/tower/BX"          , nbins, xmin, xmax)

    for c in ["good", "duplicate", "fake"]:
        hname = "pt_%s" % c
        nbins, xmin, xmax = 40, 0., 20.
        histos[hname] = TH1F(hname, "; track p_{T} [GeV]", nbins, xmin, xmax)

        hname = "eta_%s" % c
        nbins, xmin, xmax = 40, 0., 0.8
        histos[hname] = TH1F(hname, "; track #eta", nbins, xmin, xmax)

    # Change binning
    if options.pu == 0:  # single-track events
        histos["nroads_per_event"       ].SetBins(40, 0., 40.)
        histos["ncombinations_per_event"].SetBins(40, 0., 40.)
        histos["ntracks_per_event"      ].SetBins(40, 0., 40.)
        histos["ngoods_per_event"       ].SetBins(40, 0., 40.)
        histos["nduplicates_per_event"  ].SetBins(40, 0., 40.)
        histos["nfakes_per_event"       ].SetBins(40, 0., 40.)

    # Style
    for hname, h in histos.iteritems():
        h.SetLineWidth(2); h.SetMarkerSize(0)
        h.SetLineColor(col); h.SetFillColor(fcol)
    donotdelete.append(histos)
    return histos

def drawer_project(tree, histos, options):
    tree.SetBranchStatus("*", 0)
    tree.SetBranchStatus("trkParts_pt"     , 1)
    #tree.SetBranchStatus("trkParts_eta"    , 1)
    #tree.SetBranchStatus("trkParts_phi"    , 1)
    #tree.SetBranchStatus("trkParts_vx"     , 1)
    #tree.SetBranchStatus("trkParts_vy"     , 1)
    #tree.SetBranchStatus("trkParts_vz"     , 1)
    #tree.SetBranchStatus("trkParts_charge" , 1)
    #tree.SetBranchStatus("trkParts_primary", 1)
    #tree.SetBranchStatus("trkParts_signal" , 1)
    #tree.SetBranchStatus("trkParts_pdgId"  , 1)
    tree.SetBranchStatus("AMTTRoads_patternRef" , 1)
    tree.SetBranchStatus("AMTTRoads_stubRefs"   , 1)
    #tree.SetBranchStatus("AMTTTracks_invPt"     , 1)
    #tree.SetBranchStatus("AMTTTracks_phi0"      , 1)
    #tree.SetBranchStatus("AMTTTracks_cottheta"  , 1)
    #tree.SetBranchStatus("AMTTTracks_z0"        , 1)
    tree.SetBranchStatus("AMTTTracks_pt"        , 1)
    tree.SetBranchStatus("AMTTTracks_eta"       , 1)
    tree.SetBranchStatus("AMTTTracks_chi2"      , 1)
    tree.SetBranchStatus("AMTTTracks_ndof"      , 1)
    tree.SetBranchStatus("AMTTTracks_synTpId"   , 1)
    tree.SetBranchStatus("AMTTTracks_patternRef", 1)
    #tree.SetBranchStatus("AMTTTracks_stubRefs"  , 1)

    # Loop over events
    for ievt, evt in enumerate(tree):
        if (ievt == options.nentries):  break

        if (ievt % 100 == 0):  print "Processing event: %i" % ievt

        if options.pu == 0:  # single-track events
            pt = evt.trkParts_pt[0]
            if not (pt > options.minPt):
                continue

        nroads_all = evt.AMTTRoads_patternRef.size()
        nroads, ncombs = 0, 0
        for iroad in xrange(nroads_all):
            patternRef = evt.AMTTRoads_patternRef[iroad]
            if not (patternRef < options.npatterns):
                continue

            nroads += 1

            stubRefs = evt.AMTTRoads_stubRefs[iroad]
            ncombs_per_road = 1
            for v in stubRefs:
                n = v.size()
                if n != 0:
                    ncombs_per_road *= n
            ncombs += ncombs_per_road

        ntracks_all = evt.AMTTTracks_patternRef.size()
        ntracks, ngoods, nduplicates, nfakes = 0, 0, 0, 0

        for itrack in xrange(ntracks_all):
            patternRef = evt.AMTTTracks_patternRef[itrack]
            if not (patternRef < options.npatterns):
                continue

            chi2     = evt.AMTTTracks_chi2[itrack]
            ndof     = evt.AMTTTracks_ndof[itrack]
            chi2Red  = chi2/ndof

            if not (chi2Red < options.maxChi2):
                continue

            track_pt   = evt.AMTTTracks_pt      [itrack]
            track_eta  = evt.AMTTTracks_eta     [itrack]
            synTpId    = evt.AMTTTracks_synTpId [itrack]

            ntracks += 1

            if synTpId == -2:
                histos["pt_fake" ].Fill(track_pt)
                histos["eta_fake"].Fill(track_eta)
                nfakes += 1
                continue

            if synTpId == -1:
                histos["pt_duplicate" ].Fill(track_pt)
                histos["eta_duplicate"].Fill(track_eta)
                nduplicates += 1
                continue

            histos["pt_good" ].Fill(track_pt)
            histos["eta_good"].Fill(track_eta)
            ngoods += 1

        if options.verbose:  print ievt, nroads, ncombs, ntracks, ngoods, nduplicates, nfakes

        assert(ntracks == ngoods + nduplicates + nfakes)
        histos["nroads_per_event"       ].Fill(nroads)
        histos["ncombinations_per_event"].Fill(ncombs)
        histos["ntracks_per_event"      ].Fill(ntracks)
        histos["ngoods_per_event"       ].Fill(ngoods)
        histos["nduplicates_per_event"  ].Fill(nduplicates)
        histos["nfakes_per_event"       ].Fill(nfakes)

    tree.SetBranchStatus("*", 1)
    return

def drawer_draw(histos, options):
    def displayQuantiles(h, in_quantiles=[0.95,0.99], scalebox=(1.,1.)):
        # Display one-sided confidence intervals, a.k.a quantiles
        n = len(in_quantiles)
        in_quantiles = array('d', in_quantiles)
        quantiles = array('d', [0. for i in xrange(n)])
        h.GetQuantiles(n, quantiles, in_quantiles)

        gPad.Modified(); gPad.Update()
        ps = h.FindObject("stats")
        ps.SetName("mystats")

        newX1NDC = ps.GetX2NDC() - (ps.GetX2NDC() - ps.GetX1NDC()) * scalebox[0]
        newY1NDC = ps.GetY2NDC() - ((ps.GetY2NDC() - ps.GetY1NDC()) / 5 * (5 + n)) * scalebox[1]
        ps.SetX1NDC(newX1NDC)
        ps.SetY1NDC(newY1NDC)

        for iq, q in enumerate(in_quantiles):
            ps.AddText("%i%% CI = %6.4g" % (int(q*100), quantiles[iq]))
        h.stats = [h.GetMean()] + quantiles.tolist()

        h.SetStats(0)
        #gPad.Modified(); gPad.Update()
        ps.Draw()

    options.logy = True
    for hname, h in histos.iteritems():
        if options.logy:
            h.SetMaximum(h.GetMaximum() * 14); h.SetMinimum(0.5)
        else:
            h.SetMaximum(h.GetMaximum() * 1.4); h.SetMinimum(0.)

        h.Draw("hist")
        gPad.SetLogy(options.logy)
        if hname.endswith("_per_event"):
            displayQuantiles(h)

        CMS_label()
        save(options.outdir, "%s_%s" % (hname, options.ss), dot_root=True)
    return

def drawer_draw2(histos, options):

    # Specialized
    options.logy = False
    for v in ["pt", "eta"]:
        hname1 = "%s_good" % v
        hname2 = "%s_duplicate" % v
        hname3 = "%s_fake" % v

        h1 = histos[hname1]
        h2 = histos[hname2]
        h3 = histos[hname3]

        h1.SetLineColor(col1); h1.SetFillColor(fcol1)
        h2.SetLineColor(col2); h2.SetFillColor(fcol2)
        h3.SetLineColor(col3); h3.SetFillColor(fcol3)

        # Stack
        hstack1 = h1.Clone(hname1 + "_stack")
        hstack2 = h2.Clone(hname2 + "_stack")
        hstack3 = h3.Clone(hname3 + "_stack")

        hstack2.Add(hstack1)
        hstack3.Add(hstack2)

        hstack3.SetMaximum(hstack3.GetMaximum()*1.4); hstack3.SetMinimum(0)
        hstack3.SetStats(0); hstack3.Draw("hist")
        hstack2.Draw("hist same")
        hstack1.Draw("hist same")
        gPad.SetLogy(options.logy)

        moveLegend(0.64,0.82,0.94,0.94); tlegend.Clear()
        tlegend.AddEntry(h1, "good", "f")
        tlegend.AddEntry(h2, "duplicate", "f")
        tlegend.AddEntry(h3, "fake", "f")
        tlegend.Draw()

        CMS_label()
        save(options.outdir, "%s_stack_%s" % (v, options.ss), dot_root=True)

        # Ratio
        hratio1 = h1.Clone(hname1 + "_ratio")
        hratio2 = h2.Clone(hname2 + "_ratio")
        hratio3 = h3.Clone(hname3 + "_ratio")

        hratio1.Divide(hstack3)
        hratio2.Divide(hstack3)
        hratio3.Divide(hstack3)

        hratio1.SetFillStyle(0)
        hratio2.SetFillStyle(0)
        hratio3.SetFillStyle(0)

        hratio3.SetMaximum(1.1); hratio3.SetMinimum(0)
        hratio3.SetStats(0); hratio3.Draw("hist")
        hratio2.SetMaximum(1.1); hratio2.SetMinimum(0)
        hratio2.Draw("hist same")
        hratio1.SetMaximum(1.1); hratio1.SetMinimum(0)
        hratio1.Draw("hist same")
        gPad.SetLogy(options.logy)

        moveLegend(0.64,0.82,0.94,0.94); tlegend.Clear()
        tlegend.AddEntry(h1, "good", "l")
        tlegend.AddEntry(h2, "duplicate", "l")
        tlegend.AddEntry(h3, "fake", "l")
        tlegend.Draw()

        CMS_label()
        save(options.outdir, "%s_ratio_%s" % (v, options.ss), dot_root=True)

        # Norm
        hnorm1 = h1.Clone(hname1 + "_norm")
        hnorm2 = h2.Clone(hname2 + "_norm")
        hnorm3 = h3.Clone(hname3 + "_norm")

        hnorm1.Scale(1.0/hnorm1.Integral())
        hnorm2.Scale(1.0/hnorm2.Integral())
        hnorm3.Scale(1.0/hnorm3.Integral())

        hnorm1.SetFillStyle(0)
        hnorm2.SetFillStyle(0)
        hnorm3.SetFillStyle(0)

        ymax = max(hnorm1.GetMaximum(), hnorm2.GetMaximum(), hnorm3.GetMaximum())
        hnorm3.SetMaximum(ymax*1.4); hnorm3.SetMinimum(0)
        hnorm3.SetStats(0); hnorm3.Draw("hist")
        hnorm2.Draw("hist same")
        hnorm1.Draw("hist same")
        gPad.SetLogy(options.logy)

        moveLegend(0.64,0.82,0.94,0.94); tlegend.Clear()
        tlegend.AddEntry(h1, "good", "l")
        tlegend.AddEntry(h2, "duplicate", "l")
        tlegend.AddEntry(h3, "fake", "l")
        tlegend.Draw()

        CMS_label()
        save(options.outdir, "%s_norm_%s" % (v, options.ss), dot_root=True)

        donotdelete.append([hstack1, hstack2, hstack3])
        donotdelete.append([hratio1, hratio2, hratio3])
        donotdelete.append([hnorm1, hnorm2, hnorm3])
    return

def drawer_sitrep(histos, options):
    print "--- SITREP ---------------------------------------------------------"
    print "--- Variable, mean, 95%% CI, 99%% CI:"
    h = histos["nroads_per_event"]
    print "nroads per event\t{0:6.4g}\t{1:6.4g}\t{2:6.4g}".format(*h.stats)
    h = histos["ncombinations_per_event"]
    print "ncombs per event\t{0:6.4g}\t{1:6.4g}\t{2:6.4g}".format(*h.stats)
    h = histos["ntracks_per_event"]
    print "ntrks  per event\t{0:6.4g}\t{1:6.4g}\t{2:6.4g}".format(*h.stats)
    h = histos["ngoods_per_event"]
    print "ngoods per event\t{0:6.4g}\t{1:6.4g}\t{2:6.4g}".format(*h.stats)
    h = histos["nduplicates_per_event"]
    print "ndupls per event\t{0:6.4g}\t{1:6.4g}\t{2:6.4g}".format(*h.stats)
    h = histos["nfakes_per_event"]
    print "nfakes per event\t{0:6.4g}\t{1:6.4g}\t{2:6.4g}".format(*h.stats)


# ______________________________________________________________________________
# Main function
def main(options):

    # Init
    drawerInit = DrawerInit()
    tchain = TChain("ntupler/tree", "")
    tchain.AddFileInfoList(options.tfilecoll.GetList())

    # Process
    histos = drawer_book()
    drawer_project(tchain, histos, options)
    drawer_draw(histos, options)
    drawer_draw2(histos, options)
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
    parser.add_argument("--minPt", type=float, default=2, help="min pT for gen particle (default: %(default)s)")
    parser.add_argument("--maxChi2", type=float, default=5, help="max reduced chi-squared (default: %(default)s)")
    parser.add_argument("--xscale", type=float, default=1, help="scale factor for the x-axis range (default: %(default)s)")

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)

    # Call the main function
    main(options)
