#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
import re

# ______________________________________________________________________________
# Configurations

sections = {}
sections["fixed"     ] = True
sections["projective"] = False

drawerInit = DrawerInit()
gStyle.SetPadRightMargin(0.1)

imgdir = "figures_patternBank/"

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)


# ______________________________________________________________________________
# Functions

ycoverages = [0.5, 0.8, 0.9, 0.95, 1.0]

def parseLogs(ss_sizes, logname, midpoint=4294967295, every=1000000):
    dict_unsort = {}
    dict_sort = {}

    for ss_size in ss_sizes:
        with open(logname % ss_size) as f:
            unsort_data = []
            sort_data = []
            unsort_data.append((0,0))  # insert zero point
            i = every

            for l in f.readlines():
                if i <= midpoint:
                    if ("Processing event: %7u" % i) in l:
                        m = re.match("\D+(\d+)\D+(\d+)\D+(\d+)\D+(\d+\.\d+)", l)
                        if m:
                            # Use this if using muons of only one charge, set midpoint to endpoint
                            #unsort_data.append((int(m.group(3)), float(m.group(4))))
                            # Use this if using muons of both charges, set midpoint to midpoint
                            unsort_data.append((int(m.group(3))*2, float(m.group(4))))
                        i += every

                if ("Writing event: ") in l:
                    m = re.match("\D+(\d+)\D+(\d+\.\d+)", l)
                    if m:
                        sort_data.append((int(m.group(1)), float(m.group(2))))

            dict_unsort[ss_size] = unsort_data
            dict_sort[ss_size] = sort_data
    return (dict_unsort, dict_sort)

def makeGraphs(ss_sizes, dict_unsort, dict_sort):
    graphs_unsort = []
    graphs_sort = []
    for ss_size in ss_sizes:
        unsort_data = dict_unsort[ss_size]
        sort_data = dict_sort[ss_size]

        n1 = len(unsort_data)
        g1 = TGraph(n1)
        for i in xrange(n1):
            g1.SetPoint(i, unsort_data[i][0], unsort_data[i][1])
            if i == n1-1:
                print '{0:6} {1:8d} {2:5.4f}'.format(ss_size, unsort_data[i][0], unsort_data[i][1])
        graphs_unsort.append(g1)

        n2 = len(sort_data)
        g2 = TGraph(n2)
        for i in xrange(n2):
            g2.SetPoint(i, sort_data[i][0], sort_data[i][1])
        graphs_sort.append(g2)
    return (graphs_unsort, graphs_sort)


# ______________________________________________________________________________
# Fixed

if sections["fixed"]:

    def drawCoverage(ss_sizes, graphs_unsort, graphs_sort, tower="barrel", xmin=0., xmax=1e8):
        h1 = TH1F("h1_%s" % tower, "; # of patterns; running estimate for coverage", 100, xmin, xmax)
        h1.SetStats(0); h1.SetMinimum(0); h1.SetMaximum(1.2)
        h1.SetNdivisions(510, "Y")
        gStyle.SetLineStyleString(11,"36 36")
        latex.SetTextSize(0.04)

        # Draw unsorted
        h1.Draw()
        for y in ycoverages:
            line.DrawLine(xmin, y, xmax, y)

        for i, ss_size in enumerate(ss_sizes):
            g1 = graphs_unsort[i]
            g1.SetLineWidth(2); g1.SetLineStyle(11); g1.SetMarkerSize(0)
            g1.SetLineColor(paletteSet1[i])
            g1.Draw("C")

        moveLegend(0.66,0.15,0.96,0.45); legend.Clear()
        for i, ss_size in enumerate(ss_sizes):
            legend.AddEntry(graphs_unsort[i], ss_size, "l")
        legend.Draw()
        towertext = (tower[0].upper() + tower[1:] + " trigger tower")
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "fixed_%s_unsorted" % tower)

        # Redraw with log-x scale
        gPad.SetLogx()
        h1.GetXaxis().SetRangeUser(1, xmax)
        save(imgdir, "fixed_%s_unsorted_logx" % tower)
        gPad.SetLogx(0)
        h1.GetXaxis().SetRangeUser(0, xmax)

        # Draw sorted
        h1.Draw()
        for y in ycoverages:
            line.DrawLine(xmin, y, xmax, y)

        for i, ss_size in enumerate(ss_sizes):
            g1 = graphs_unsort[i]
            g1.SetLineColor(kGray)
            g1.Draw("C")

        for i, ss_size in enumerate(ss_sizes):
            g2 = graphs_sort[i]
            g2.SetLineWidth(2); g2.SetLineStyle(1); g2.SetMarkerSize(0)
            g2.SetLineColor(paletteSet1[i])
            g2.Draw("C")

        moveLegend(0.66,0.15,0.96,0.45); legend.Clear()
        for i, ss_size in enumerate(ss_sizes):
            legend.AddEntry(graphs_sort[i], ss_size, "l")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "fixed_%s_sorted" % tower)

        gPad.SetLogx()
        h1.GetXaxis().SetRangeUser(1, xmax)
        save(imgdir, "fixed_%s_sorted_logx" % tower)
        gPad.SetLogx(0)
        h1.GetXaxis().SetRangeUser(0, xmax)


    #ss_sizes = ["ss1024"]
    ss_sizes = ["ss32", "ss64", "ss128", "ss256", "ss512", "ss1024"]
    logs = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_results/"

    # Barrel
    logname = "patternBank_sp16_%s_tt27_200M.log"
    dict_unsort, dict_sort = parseLogs(ss_sizes, logs + "/" + logname, 100000000)
    graphs_unsort, graphs_sort = makeGraphs(ss_sizes, dict_unsort, dict_sort)
    drawCoverage(ss_sizes, graphs_unsort, graphs_sort, tower="barrel", xmin=0., xmax=7e7)

    # Hybrid
    logname = "patternBank_sp16_%s_tt35_200M.log"
    dict_unsort, dict_sort = parseLogs(ss_sizes, logs + "/" + logname, 100000000)
    graphs_unsort, graphs_sort = makeGraphs(ss_sizes, dict_unsort, dict_sort)
    drawCoverage(ss_sizes, graphs_unsort, graphs_sort, tower="hybrid", xmin=0., xmax=7e7)

    # Endcap
    logname = "patternBank_sp16_%s_tt43_200M.log"
    dict_unsort, dict_sort = parseLogs(ss_sizes, logs + "/" + logname, 100000000)
    graphs_unsort, graphs_sort = makeGraphs(ss_sizes, dict_unsort, dict_sort)
    drawCoverage(ss_sizes, graphs_unsort, graphs_sort, tower="endcap", xmin=0., xmax=7e7)

# ______________________________________________________________________________
# Projective

if sections["projective"]:

    def drawCoverage(ss_sizes, graphs_unsort, graphs_sort, tower="barrel", xmin=0., xmax=1e8):
        h1 = TH1F("h1_%s" % tower, "; # of patterns; running estimate for coverage", 100, xmin, xmax)
        h1.SetStats(0); h1.SetMinimum(0); h1.SetMaximum(1.2)
        h1.SetNdivisions(510, "Y")
        gStyle.SetLineStyleString(11,"36 36")
        latex.SetTextSize(0.04)

        # Draw unsorted
        h1.Draw()
        for y in ycoverages:
            line.DrawLine(xmin, y, xmax, y)

        for i, ss_size in enumerate(ss_sizes):
            g1 = graphs_unsort[i]
            g1.SetLineWidth(2); g1.SetLineStyle(11); g1.SetMarkerSize(0)
            g1.SetLineColor(paletteSet1[i])
            g1.Draw("C")

        moveLegend(0.66,0.15,0.96,0.45); legend.Clear()
        for i, ss_size in enumerate(ss_sizes):
            legend.AddEntry(graphs_unsort[i], ss_size, "l")
        legend.Draw()
        towertext = (tower[0].upper() + tower[1:] + " trigger tower")
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "projective_%s_unsorted" % tower)

        # Redraw with log-x scale
        gPad.SetLogx()
        h1.GetXaxis().SetRangeUser(1, xmax)
        save(imgdir, "projective_%s_unsorted_logx" % tower)
        gPad.SetLogx(0)
        h1.GetXaxis().SetRangeUser(0, xmax)

        # Draw sorted
        h1.Draw()
        for y in ycoverages:
            line.DrawLine(xmin, y, xmax, y)

        for i, ss_size in enumerate(ss_sizes):
            g1 = graphs_unsort[i]
            g1.SetLineColor(kGray)
            g1.Draw("C")

        for i, ss_size in enumerate(ss_sizes):
            g2 = graphs_sort[i]
            g2.SetLineWidth(2); g2.SetLineStyle(1); g2.SetMarkerSize(0)
            g2.SetLineColor(paletteSet1[i])
            g2.Draw("C")

        moveLegend(0.66,0.15,0.96,0.45); legend.Clear()
        for i, ss_size in enumerate(ss_sizes):
            legend.AddEntry(graphs_sort[i], ss_size, "l")
        legend.Draw()
        latex.DrawLatex(0.56, 0.88, towertext)
        CMS_label()
        save(imgdir, "projective_%s_sorted" % tower)

        gPad.SetLogx()
        h1.GetXaxis().SetRangeUser(1, xmax)
        save(imgdir, "projective_%s_sorted_logx" % tower)
        gPad.SetLogx(0)
        h1.GetXaxis().SetRangeUser(0, xmax)


    #ss_sizes = ["lu400x1"]
    ss_sizes = ["lu600x1", "lu400x1", "lu200x1", "lu100x2", "lu50x4", "lu20x10"]
    logs = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_results/"

    # Barrel
    logname = "patternBank_%s_tt27_200M.log"
    dict_unsort, dict_sort = parseLogs(ss_sizes, logs + "/" + logname, 100000000)
    graphs_unsort, graphs_sort = makeGraphs(ss_sizes, dict_unsort, dict_sort)
    drawCoverage(ss_sizes, graphs_unsort, graphs_sort, tower="barrel", xmin=0., xmax=7e7)

