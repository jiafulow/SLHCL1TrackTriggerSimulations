#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
from array import array
import numpy as np
import os

# ______________________________________________________________________________
# Configurations

sections = {}
sections["fixed"     ] = True
sections["projective"] = False
sections["fixme"     ] = False

drawerInit = DrawerInit()
gStyle.SetPadRightMargin(0.1)

EOS = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_results_PU140/"
DATE = "20141108"

fixed_settings_tt27 = [
    ("ss32"  ,   42222520,  0.9034),
    ("ss64"  ,   16960498,  0.9725),
    ("ss128" ,    6240499,  0.9931),
    ("ss256" ,    2042708,  0.9983),
    ("ss512" ,     637157,  0.9997),
    ("ss1024",     222227,  0.9999),
]

projective_settings_tt27 = [
    ("600x0" ,    15842589, 0.9758),
    ("400x0" ,     4313637, 0.9953),
    ("200x0" ,      565637, 0.9995),
    ("200x1" ,     3217636, 0.9954),
    ("100x2" ,     1633571, 0.9979),
    ("20x10" ,     1146025, 0.9989),
]

imgdir = "figures_roads/"

# Number of events
nentries = 200
#nentries = 20

chain = TChain("ntupler/tree", "")

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)

# ______________________________________________________________________________
# Load

def listdir_fullpath(d):
    return [os.path.join(d, f) for f in os.listdir(d) if f.endswith(".root")]


# ______________________________________________________________________________
# Fixed

if sections["fixed"]:

    gStyle.SetTitleSize(0.05, "Y")
    latex.SetTextSize(0.05)

    settings = fixed_settings_tt27

    def bookRate(results):
        superstrips = {}
        graphs = []
        graphs2 = []

        for ss, npatterns, coverage in settings:
            chain.Reset()
            infiles = listdir_fullpath(EOS + "/" + (results % ss) + "_" + DATE)
            for f in infiles:
                chain.Add(f)

            # Sample 200 points
            npoints = 200
            every = npatterns / npoints
            xvalues, yvalues, zvalues = [], [], []
            x, y, z = 0, 0, 0
            for i in xrange(0,npatterns,every):
                xvalues.append(i); yvalues.append(y); zvalues.append(z)
            xvalues.append(npatterns-1); yvalues.append(y); zvalues.append(z)
            npoints = len(xvalues)

            yyvalues, zzvalues = [], []
            for i in xrange(npoints):
                yyvalues.append([]); zzvalues.append([])

            # Loop over events
            tree = chain
            tree.SetBranchStatus("*", 0)
            tree.SetBranchStatus("AMTTRoads_nSuperstrips"    , 1)
            tree.SetBranchStatus("AMTTRoads_bankIndex"       , 1)
            tree.SetBranchStatus("AMTTRoads_hitSuperstripIds", 1)

            for i_ievt, ievt in enumerate(tree):
                if (i_ievt == nentries):  break

                # Init counters
                for i in xrange(npoints):
                    yyvalues[i].append(0)
                    zzvalues[i].append(0)

                for nSuperstrips, bankIndex, superstripIds in izip(ievt.AMTTRoads_nSuperstrips, ievt.AMTTRoads_bankIndex, ievt.AMTTRoads_hitSuperstripIds):
                    # Calculate # of combinations
                    ssidmap = {}
                    for ssid in superstripIds:
                        ssidmap[ssid] = ssidmap.get(ssid, 0) + 1
                    assert(nSuperstrips == len(ssidmap))

                    ncombinations_per_road = 1
                    for k, v in ssidmap.iteritems():
                        ncombinations_per_road *= v

                    # Sum # of roads and # of combinations
                    for i in xrange(npoints):
                        if bankIndex < xvalues[i]:
                            yyvalues[i][-1] += 1
                            zzvalues[i][-1] += ncombinations_per_road

                assert(yyvalues[-1][-1] == len(ievt.AMTTRoads_bankIndex))

            # Find mean
            for i in xrange(npoints):
                yvalues[i] = np.mean(yyvalues[i])
                zvalues[i] = np.mean(zzvalues[i])

            for x, y, z in zip(xvalues, yvalues, zvalues):
                print "..", x, y, z

            superstrips[ss] = (npatterns, yvalues[-1], zvalues[-1])
            print ss, superstrips[ss]

            gr = TGraph(npoints, array('d', xvalues), array('d', yvalues))
            graphs.append(gr)

            gr = TGraph(npoints, array('d', xvalues), array('d', zvalues))
            graphs2.append(gr)

            tree.SetBranchStatus("*", 0)
        return (superstrips, graphs, graphs2)

    def drawRate(superstrips, graphs, graphs2, xmin=0, xmax=1e8, tower="tt27"):
        #ymax = max([s[1] for s in superstrips.values()]) * 1.2
        #zmax = max([s[2] for s in superstrips.values()]) * 1.2

        ymax = superstrips["ss256"][1] * 1.2
        zmax = superstrips["ss256"][2] * 1.2

        hframe = TH1F("hframe", "; # of patterns; <# of roads>", 100, xmin, xmax)
        hframe.SetStats(0); hframe.SetMinimum(0); hframe.SetMaximum(ymax)
        hframe.SetNdivisions(510, "Y")

        # Style
        for i, gr in enumerate(graphs):
            gr.SetLineWidth(2); gr.SetLineStyle(1); gr.SetMarkerSize(0)
            gr.SetLineColor(paletteSet1[i])

        # Draw
        hframe.Draw()
        for i, gr in enumerate(graphs):
            gr.Draw("C")

        # Legend
        moveLegend(0.66,0.15,0.96,0.45); legend.Clear()
        for i, gr in enumerate(graphs):
            ss = settings[i][0]
            legend.AddEntry(gr, ss, "l")
            gr.SetName("gr_" + ss)
        legend.Draw()
        if "tt27" in tower:
            towertext = "Barrel"
        elif "tt35" in tower:
            towertext = "Hybrid"
        elif "tt43" in tower:
            towertext = "Endcap"
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "fixed_roads_%s" % tower, dot_root=True)

        # Zoom in
        hframe.Draw()
        hframe.GetXaxis().SetRangeUser(0, xmax/50)
        for i, gr in enumerate(graphs):
            gr.Draw("C")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "fixed_roads_zoom_%s" % tower)

        hframe2 = TH1F("hframe2", "; # of patterns; <# of fit combinations>", 100, xmin, xmax)
        hframe2.SetStats(0); hframe2.SetMinimum(0); hframe2.SetMaximum(zmax)
        hframe2.SetNdivisions(510, "Y")

        # Style
        for i, gr in enumerate(graphs2):
            gr.SetLineWidth(2); gr.SetLineStyle(1); gr.SetMarkerSize(0)
            gr.SetLineColor(paletteSet1[i])

        # Draw
        hframe2.Draw()
        for i, gr in enumerate(graphs2):
            gr.Draw("C")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "fixed_fits_%s" % tower, dot_root=True)

        # Zoom in
        hframe2.Draw()
        hframe2.GetXaxis().SetRangeUser(0, xmax/50)
        for i, gr in enumerate(graphs2):
            gr.Draw("C")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "fixed_fits_zoom_%s" % tower)

        donotdelete = [hframe, hframe2]
        return donotdelete


    # Barrel 2 GeV
    results = "Neutrino_sp16_%s_tt27"
    (superstrips, graphs, graphs2) = bookRate(results)
    d = drawRate(superstrips, graphs, graphs2, xmax=5e7, tower="tt27")

    # Barrel 3 GeV
    #results = "Neutrino_sp16_%s_tt27_pt3"
    #(superstrips, graphs, graphs2) = bookRate(results)
    #d = drawRate(superstrips, graphs, graphs2, xmax=5e7, tower="tt27_pt3")


# ______________________________________________________________________________
# Projective

if sections["projective"]:

    gStyle.SetTitleSize(0.05, "Y")
    latex.SetTextSize(0.05)

    settings = projective_settings_tt27

    def bookRate(results):
        superstrips = {}
        graphs = []
        graphs2 = []

        for ss, npatterns, coverage in settings:
            chain.Reset()
            infiles = listdir_fullpath(EOS + "/" + (results % ss) + "_" + DATE)
            for f in infiles:
                chain.Add(f)

            # Sample 200 points
            npoints = 200
            every = npatterns / npoints
            xvalues, yvalues, zvalues = [], [], []
            x, y, z = 0, 0, 0
            for i in xrange(0,npatterns,every):
                xvalues.append(i); yvalues.append(y); zvalues.append(z)
            xvalues.append(npatterns-1); yvalues.append(y); zvalues.append(z)
            npoints = len(xvalues)

            yyvalues, zzvalues = [], []
            for i in xrange(npoints):
                yyvalues.append([]); zzvalues.append([])

            # Loop over events
            tree = chain
            tree.SetBranchStatus("*", 0)
            tree.SetBranchStatus("AMTTRoads_nSuperstrips"    , 1)
            tree.SetBranchStatus("AMTTRoads_bankIndex"       , 1)
            tree.SetBranchStatus("AMTTRoads_hitSuperstripIds", 1)

            for i_ievt, ievt in enumerate(tree):
                if (i_ievt == nentries):  break

                # Init counters
                for i in xrange(npoints):
                    yyvalues[i].append(0)
                    zzvalues[i].append(0)

                for nSuperstrips, bankIndex, superstripIds in izip(ievt.AMTTRoads_nSuperstrips, ievt.AMTTRoads_bankIndex, ievt.AMTTRoads_hitSuperstripIds):
                    # Calculate # of combinations
                    ssidmap = {}
                    for ssid in superstripIds:
                        ssidmap[ssid] = ssidmap.get(ssid, 0) + 1
                    assert(nSuperstrips == len(ssidmap))

                    ncombinations_per_road = 1
                    for k, v in ssidmap.iteritems():
                        ncombinations_per_road *= v

                    # Sum # of roads and # of combinations
                    for i in xrange(npoints):
                        if bankIndex < xvalues[i]:
                            yyvalues[i][-1] += 1
                            zzvalues[i][-1] += ncombinations_per_road

                assert(yyvalues[-1][-1] == len(ievt.AMTTRoads_bankIndex))

            # Find mean
            for i in xrange(npoints):
                yvalues[i] = np.mean(yyvalues[i])
                zvalues[i] = np.mean(zzvalues[i])

            for x, y, z in zip(xvalues, yvalues, zvalues):
                print "..", x, y, z

            superstrips[ss] = (npatterns, yvalues[-1], zvalues[-1])
            print ss, superstrips[ss]

            gr = TGraph(npoints, array('d', xvalues), array('d', yvalues))
            graphs.append(gr)

            gr = TGraph(npoints, array('d', xvalues), array('d', zvalues))
            graphs2.append(gr)

            tree.SetBranchStatus("*", 0)
        return (superstrips, graphs, graphs2)

    def drawRate(superstrips, graphs, graphs2, xmin=0, xmax=1e8, tower="tt27"):
        #ymax = max([s[1] for s in superstrips.values()]) * 1.2
        #zmax = max([s[2] for s in superstrips.values()]) * 1.2

        ymax = superstrips["400x0"][1] * 1.2
        zmax = superstrips["400x0"][2] * 1.2

        hframe = TH1F("hframe", "; # of patterns; <# of roads>", 100, xmin, xmax)
        hframe.SetStats(0); hframe.SetMinimum(0); hframe.SetMaximum(ymax)
        hframe.SetNdivisions(510, "Y")

        # Style
        for i, gr in enumerate(graphs):
            gr.SetLineWidth(2); gr.SetLineStyle(1); gr.SetMarkerSize(0)
            gr.SetLineColor(paletteSet1[i])

        # Draw
        hframe.Draw()
        for i, gr in enumerate(graphs):
            gr.Draw("C")

        # Legend
        moveLegend(0.66,0.15,0.96,0.45); legend.Clear()
        for i, gr in enumerate(graphs):
            ss = settings[i][0]
            legend.AddEntry(gr, ss, "l")
            gr.SetName("gr_" + ss)
        legend.Draw()
        if "tt27" in tower:
            towertext = "Barrel"
        elif "tt35" in tower:
            towertext = "Hybrid"
        elif "tt43" in tower:
            towertext = "Endcap"
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "projective_roads_%s" % tower, dot_root=True)

        # Zoom in
        hframe.Draw()
        hframe.GetXaxis().SetRangeUser(0, xmax/50)
        for i, gr in enumerate(graphs):
            gr.Draw("C")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "projective_roads_zoom_%s" % tower)

        hframe2 = TH1F("hframe2", "; # of patterns; <# of fit combinations>", 100, xmin, xmax)
        hframe2.SetStats(0); hframe2.SetMinimum(0); hframe2.SetMaximum(zmax)
        hframe2.SetNdivisions(510, "Y")

        # Style
        for i, gr in enumerate(graphs2):
            gr.SetLineWidth(2); gr.SetLineStyle(1); gr.SetMarkerSize(0)
            gr.SetLineColor(paletteSet1[i])

        # Draw
        hframe2.Draw()
        for i, gr in enumerate(graphs2):
            gr.Draw("C")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "projective_fits_%s" % tower, dot_root=True)

        # Zoom in
        hframe2.Draw()
        hframe2.GetXaxis().SetRangeUser(0, xmax/50)
        for i, gr in enumerate(graphs2):
            gr.Draw("C")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "projective_fits_zoom_%s" % tower)

        donotdelete = [hframe, hframe2]
        return donotdelete


    # Barrel 2 GeV
    results = "Neutrino_lu%s_tt27"
    (superstrips, graphs, graphs2) = bookRate(results)
    d = drawRate(superstrips, graphs, graphs2, xmax=5e7, tower="tt27")

    # Barrel 3 GeV
    #results = "Neutrino_lu%s_tt27_pt3"
    #(superstrips, graphs, graphs2) = bookRate(results)
    #d = drawRate(superstrips, graphs, graphs2, xmax=5e7, tower="tt27_pt3")


# ______________________________________________________________________________
if sections["fixme"]:
    pass

