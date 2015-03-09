#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
from array import array
from math import sqrt, pi, sin, cos, tan, sinh, exp, atan


# ______________________________________________________________________________
# Configurations

sections = {}
sections["control"     ] = True

drawerInit = DrawerInit()
gStyle.SetNumberContours(100)
gStyle.SetPalette(55)  # rainbow color map

imgdir = "figures_scattering/"

# Number of events
nentries = 100000
#nentries = 1000
#nentries = 10

chain = TChain("ntupler/tree", "")

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)

# ______________________________________________________________________________
# Load

ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)


# ______________________________________________________________________________
# Control

if sections["control"]:

    from ROOT import TPolyLine
    gStyle.SetLabelSize(0.05, "Y")

    latex2 = TLatex()
    latex2.SetNDC(0)
    latex2.SetTextFont(42)
    latex2.SetTextSize(0.02)

    line2 = TLine()
    line2.SetLineColor(kGray+3)
    line2.SetLineStyle(1)
    line2.SetLineWidth(2)

    nbinsx, xmin, xmax = 1500/5, -40., 110.
    nbinsy, ymin, ymax = 1500/5,   0., 150.
    nbinsz, zmin, zmax = 1500/5, -30., 120.

    vertexmap = json.load(open("../data/module_vertices.json"), object_pairs_hook=convert_key_to_int)

    rmeans = b_rcoord_luciano_m

    def clone_tpolyline(l):
        n, x, y = l.GetN(), l.GetX(), l.GetY()
        x2, y2 = [], []
        for i in xrange(n):
            x2.append(x[i]/10.)
            y2.append(y[i]/10.)
        ll = TPolyLine(n, array('d', x2), array('d', y2))
        return ll

    def get_tpolylines(imgfile):
        tfile = TFile.Open(imgfile)
        img = tfile.Get(imgfile[:-5])
        tpolylines = []
        for l in img.GetListOfPrimitives():
            if l.GetName() == "TPolyLine":
                ll = clone_tpolyline(l)
                ll.SetLineStyle(l.GetLineStyle()); ll.SetLineWidth(l.GetLineWidth()); ll.SetLineColor(l.GetLineColor())
                tpolylines.append(ll)
        return tpolylines

    def make_tpolyline(l, view):
        xy = set()
        for i in xrange(4):
            c = l[3*i:3*(i+1)]
            if view == "XY":
                c = (c[0], c[1])
            elif view == "ZR":
                c = (c[2], sqrt(c[0]*c[0] + c[1]*c[1]))
            xy.add(c)
        n, x, y = 0, [], []
        for c in xy:
            x.append(c[0])
            y.append(c[1])
            n += 1
        if n == 4:  # close the poly line in case it's made of 4 distinct points
            x.append(x[0])
            y.append(y[0])
            n += 1
        ll = TPolyLine(n, array('d', x), array('d', y))
        return ll

    def get_tpolylines2(view):
        tpolylines = []
        if view == "XY":
            towers = [24, 25, 26, 28, 29, 30, 31,  27]  # put 27 last
        elif view == "ZR":
            towers = [3, 11, 19, 35, 43, 27]  # put 27 last
        for tt in towers:
            for k in ttmap[tt]:
                for kk in [k, -k]:
                    ll = make_tpolyline(vertexmap[kk], view)
                    ll.SetLineStyle(1); ll.SetLineWidth(2)
                    if tt == 27:
                        if isPSModule(k):
                            ll.SetLineColor(palette[0])
                        else:
                            ll.SetLineColor(palette[1])
                    else:
                        if isPSModule(k):
                            ll.SetLineColor(lightpalette[0])
                        else:
                            ll.SetLineColor(lightpalette[1])
                    tpolylines.append(ll)
        return tpolylines

    def get_tgraphs(pt):
        simEta = 1.1/3
        simPhi = pi*3/8

        n = 50
        xp, yp, xm, ym = [], [], [], []
        zz, rr = [], []
        for i in xrange(n+1):
            r = (ymax - ymin)/n * i
            deltaPhi = - 0.3*3.8*r*1e-2 / float(pt) / 2.0
            xp.append(r * cos(simPhi + deltaPhi))
            xm.append(r * cos(simPhi - deltaPhi))
            yp.append(r * sin(simPhi + deltaPhi))
            ym.append(r * sin(simPhi - deltaPhi))
            zz.append(r * sinh(simEta))
            rr.append(r)
        n += 1

        gp = TGraph(n, array('d', xp), array('d', yp))
        gm = TGraph(n, array('d', xm), array('d', ym))
        gzr = TGraph(n, array('d', zz), array('d', rr))
        tgraphs = [gp, gm, gzr]
        for g in tgraphs:
            g.SetLineStyle(1); g.SetLineWidth(2); g.SetLineColor(palette[3])
        return tgraphs


    def project(infile, infilelu, pt):
        # Book TH2F
        hxy1 = TH2F("hxy1_pt%i" % pt, "; x [cm]; y [cm]", nbinsx, xmin, xmax, nbinsy, ymin, ymax)
        hzr1 = TH2F("hzr1_pt%i" % pt, "; z [cm]; r [cm]", nbinsz, zmin, zmax, nbinsy, ymin, ymax)
        hxy2 = hxy1.Clone("hxy2_pt%i" % pt)
        hzr2 = hzr1.Clone("hzr2_pt%i" % pt)
        histos = [hxy1, hxy2, hzr1, hzr2]

        # Get TPolyLine's
        #tpolylines_xy = get_tpolylines("img001.root")
        #tpolylines_zr = get_tpolylines("img000.root")
        tpolylines_xy = get_tpolylines2("XY")
        tpolylines_zr = get_tpolylines2("ZR")

        # Get TGraphs
        tgraphs = get_tgraphs(pt)

        # Get TTStubs
        chain.Reset()
        chain.Add(infile)

        # Project
        tree = chain
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("TTStubs_r"  , 1)
        tree.SetBranchStatus("TTStubs_phi", 1)
        tree.SetBranchStatus("TTStubs_z"  , 1)

        j_ievt = 0
        for i_ievt, ievt in enumerate(tree):
            if (j_ievt == nentries):  break

            if len(ievt.TTStubs_r) == 6:
                for r, phi, z in izip(ievt.TTStubs_r, ievt.TTStubs_phi, ievt.TTStubs_z):
                    hxy1.Fill(r*cos(phi), r*sin(phi))
                    hzr1.Fill(z, r)
                j_ievt += 1

        # Read from Luciano
        j_ievt = 0
        with open(infilelu) as f:
            for line in f:
                if (j_ievt == nentries):  break

                stubs = line.split(",")
                assert(len(stubs) == 13)
                stubs = stubs[1:]
                for i, r in enumerate(rmeans):
                    phi = float(stubs[2*i]) / rmeans[i]
                    z = float(stubs[2*i+1])
                    hxy2.Fill(r*cos(phi)*1e2, r*sin(phi)*1e2)
                    hzr2.Fill(z*1e2, r*1e2)
                j_ievt += 1

        tree.SetBranchStatus("*", 1)
        return [histos, tpolylines_xy, tpolylines_zr, tgraphs]

    def drawXY(histos, tpolylines, tgraphs, imgdir):
        h1, h2 = histos[0], histos[1]
        pt = int(h1.GetName()[7:])

        # Style
        h1.SetMarkerSize(1.0); h1.SetMarkerStyle(5)
        h2.SetMarkerSize(1.0); h2.SetMarkerStyle(5)

        #c1 = TCanvas()  # overwrite canvas from imgfile
        hframe = h1.Clone("hframe")
        hframe.Reset(); hframe.SetLineColor(kWhite)
        hframe.SetStats(0); hframe.Draw()

        for l in tpolylines:
            l.Draw()
        for g in tgraphs[0:2]:
            g.Draw("l")
        for phi in [pi/4, pi/2]:
            if phi < 1.0:
                line2.DrawLine(0, 0, xmax, xmax * tan(phi))
            else:
                line2.DrawLine(0, 0, ymax/tan(phi), ymax)

        moveLegend(0.58,0.80,0.94,0.92); legend.Clear()
        legend.AddEntry(hframe, "p_{T,0} = %i GeV" % pt, "l")
        legend.AddEntry(hframe, "#eta_{0}  = 1.1/3", "l")
        legend.AddEntry(hframe, "#phi_{0}  = 3#pi/8", "l")
        legend.Draw()
        h1.Draw("same")
        CMS_label()
        save(imgdir, "scattering_control_xy1_pt%i" % pt)

        # Read from Luciano
        hframe.Draw()
        for l in tpolylines:
            l.Draw()
        for g in tgraphs[0:2]:
            g.Draw("l")
        for phi in [pi/4, pi/2]:
            if phi < 1.0:
                line2.DrawLine(0, 0, xmax, xmax * tan(phi))
            else:
                line2.DrawLine(0, 0, ymax/tan(phi), ymax)
        legend.Draw()
        h2.Draw("same")
        CMS_label()
        save(imgdir, "scattering_control_xy2_pt%i" % pt)

        donotdelete = [hframe]
        return donotdelete

    def drawZR(histos, tpolylines, tgraphs, imgdir):
        h1, h2 = histos[2], histos[3]
        pt = int(h1.GetName()[7:])

        # Style
        h1.SetMarkerSize(1.0); h1.SetMarkerStyle(5)
        h2.SetMarkerSize(1.0); h2.SetMarkerStyle(5)

        #c1 = TCanvas()  # overwrite canvas from imgfile
        hframe = h1.Clone("hframe")
        hframe.Reset(); hframe.SetLineColor(kWhite)
        hframe.SetStats(0); hframe.Draw()

        for l in tpolylines:
            l.Draw()
        for g in tgraphs[2:3]:
            g.Draw("l")
        for eta in [0, 2.2/3]:
            theta = 2.0 * atan(exp(-eta))
            if theta < 1.0:
                line2.DrawLine(0, 0, zmax, zmax*tan(theta))
            else:
                line2.DrawLine(0, 0, ymax/tan(theta), ymax)

        moveLegend(0.58,0.80,0.94,0.92); legend.Clear()
        legend.AddEntry(hframe, "p_{T,0} = %i GeV" % pt, "l")
        legend.AddEntry(hframe, "#eta_{0}  = 1.1/3", "l")
        legend.AddEntry(hframe, "#phi_{0}  = 3#pi/8", "l")
        legend.Draw()
        h1.Draw("same")
        CMS_label()
        save(imgdir, "scattering_control_zr1_pt%i" % pt)

        # Read from Luciano
        hframe.Draw()
        for l in tpolylines:
            l.Draw()
        for g in tgraphs[2:3]:
            g.Draw("l")
        for eta in [0, 2.2/3]:
            theta = 2.0 * atan(exp(-eta))
            if theta < 1.0:
                line2.DrawLine(0, 0, zmax, zmax*tan(theta))
            else:
                line2.DrawLine(0, 0, ymax/tan(theta), ymax)
        legend.Draw()
        h2.Draw("same")
        CMS_label()
        save(imgdir, "scattering_control_zr2_pt%i" % pt)

        donotdelete = [hframe]
        return donotdelete


    indir = "/uscms_data/d2/jiafu/L1TrackTrigger/CMSSW_6_2_0_SLHC12_patch1/src/SLHCL1TrackTriggerSimulations/Configuration/test/control/"
    infile = indir + "stubs_barrel_pt%i.1.root"
    indirlu = "/uscms_data/d2/jiafu/L1TrackTrigger/external/fromLuciano/LucianosToy-v0.3/jftest/"
    infilelu = indirlu + "hits_pt%i.txt"

    pt = 2
    histos, tpolylines_xy, tpolylines_zr, tgraphs = project(infile % pt, infilelu % pt, pt)
    d = drawXY(histos, tpolylines_xy, tgraphs, imgdir)
    d = drawZR(histos, tpolylines_zr, tgraphs, imgdir)

    pt = 3
    histos, tpolylines_xy, tpolylines_zr, tgraphs = project(infile % pt, infilelu % pt, pt)
    d = drawXY(histos, tpolylines_xy, tgraphs, imgdir)
    d = drawZR(histos, tpolylines_zr, tgraphs, imgdir)

    pt = 5
    histos, tpolylines_xy, tpolylines_zr, tgraphs = project(infile % pt, infilelu % pt, pt)
    d = drawXY(histos, tpolylines_xy, tgraphs, imgdir)
    d = drawZR(histos, tpolylines_zr, tgraphs, imgdir)

    pt = 10
    histos, tpolylines_xy, tpolylines_zr, tgraphs = project(infile % pt, infilelu % pt, pt)
    d = drawXY(histos, tpolylines_xy, tgraphs, imgdir)
    d = drawZR(histos, tpolylines_zr, tgraphs, imgdir)

