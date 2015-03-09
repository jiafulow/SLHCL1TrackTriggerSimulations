#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
from math import pi
import itertools
import os

# ______________________________________________________________________________
# Configurations

samples = {}
samples["nu140"       ] = False
samples["tttt140"     ] = False
samples["tt27"        ] = False
samples["tt27_nu140"  ] = False
samples["tt27_tttt140"] = True

sections = {}
sections["coverage"     ] = False
sections["roads"        ] = False
sections["summary"      ] = True

drawerInit = DrawerInit()
gStyle.SetNumberContours(100)
gStyle.SetPalette(55)  # rainbow color map


#DATE = "20141108"
DATE = "20141222"

settings = [
    ("sp16_ss256" , 87631104,  0.9999),
]

#settings_tt27 = [
#    ("sp16_ss32"  , 54214494,  0.9452),
#    ("sp16_ss64"  , 20262384,  0.9854),
#    ("sp16_ss128" ,  7103498,  0.9966),
#    ("sp16_ss256" ,  2241139,  0.9992),
#    ("sp16_ss512" ,   676611,  0.9999),
#    ("sp16_ss1024",   227105,  1.0000),
#]

#settings_tt27 = [
#    ("sp16_ss64"  ,  5002400,  0.9),
#    ("sp16_ss128" ,  1314880,  0.9),
#    ("sp16_ss256" ,   344890,  0.9),
#    ("sp16_ss512" ,    92530,  0.9),
#    ("sp16_ss1024",    27940,  0.9),
#]

#settings_tt27 = [
#    ("lu600x0"  ,  2033820,  0.9),
#    ("lu400x0"  ,   486640,  0.9),
#    ("lu200x0"  ,    70130,  0.9),
#    ("lu200x1"  ,   295470,  0.9),
#    ("lu100x2"  ,   205440,  0.9),
#    ("lu20x10"  ,   198990,  0.9),
#]

settings_tt27 = [
    ("sf0p5_nz1",   275290,  0.9),
    ("sf0p7_nz1",   107500,  0.9),
    ("sf0p7_nz2",   590260,  0.9),
    ("sf1_nz1"  ,    44810,  0.9),
    ("sf1_nz2"  ,   252920,  0.9),
    ("sf1_nz4"  ,  1180040,  0.9),
    ("sf2_nz1"  ,     9730,  0.9),
    ("sf2_nz2"  ,    52720,  0.9),
    ("sf2_nz4"  ,   270890,  0.9),
]

EOS = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_results/"
results = "SingleMuPlusMinus_%s"

col  = TColor.GetColor("#1f78b4")  # Paired
fcol = TColor.GetColor("#a6cee3")
imgdir = "figures_amsim/"

if samples["tt27"]:
    settings = settings_tt27
    results = "SingleMuPlusMinus_%s_tt27"
    imgdir = "figures_amsim_tt27/"

if samples["nu140"] or samples["tt27_nu140"]:
    EOS  = EOS.replace("_results","_results_PU140")
    col  = TColor.GetColor("#e31a1c")  # Paired
    fcol = TColor.GetColor("#fb9a99")

    if samples["nu140"]:
        results = "Neutrino_%s"
        imgdir = "figures_amsim_nu140/"
    else:
        settings = settings_tt27
        results = "Neutrino_%s_tt27"
        imgdir = "figures_amsim_nu140_tt27/"

if samples["tttt140"] or samples["tt27_tttt140"]:
    EOS  = EOS.replace("_results","_results_PU140")
    col  = TColor.GetColor("#6a3d9a")  # Paired
    fcol = TColor.GetColor("#cab2d6")

    if samples["tttt140"]:
        results = "TTbarTTbar_%s"
        imgdir = "figures_amsim_tttt140/"
    else:
        settings = settings_tt27
        #results = "TTbarTTbar_%s_tt27"
        results = "TTbarTTbar_%s_tt27_pt3"
        imgdir = "figures_amsim_tttt140_tt27/"


# Number of events
nentries = 2000000
#nentries = 20000

chain = TChain("ntupler/tree", "")

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)

# ______________________________________________________________________________
# Load

ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)

def listdir_fullpath(d):
    return [os.path.join(d, f) for f in os.listdir(d) if f.endswith(".root")]

def parse_ss_to_setting(ss):
    if "sp" in ss:
        ss = ss.replace("sp16_", "")
    elif "lu" in ss:
        ss = ss.replace("lu", "")
    elif "sf" in ss:
        ss = ss.replace("sf", "sf=").replace("_nz", ",nz=")
        ss = ss.replace("0p", "0.")
    ss = ss.replace("_", " ")
    return ss


# ______________________________________________________________________________
# Coverage

if sections["coverage"]:

    latex.SetTextSize(0.036)
    if samples["nu140"] or samples["tt27_nu140"] or samples["tttt140"] or samples["tt27_tttt140"]:
        print "ERROR!"
        exit

    def bookCoverage():
        histos = {}

        # 1D: pt, pt (zoom), eta, phi, vz, charge
        ytitle = "coverage"
        # NOTE: what is previously called "acceptance" has been changed to "AM PR efficiency"
        #       but the "ac_" prefix is still used in code
        #ac_ytitle = "acceptance"
        ac_ytitle = "#epsilon_{AM PR}"
        for i in xrange(3):
            hname = "pt_%i" % i
            histos[hname] = TProfile(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 200., 0., 2., "")
            histos["ac_"+hname] = TProfile("ac_"+hname, "; p_{T} [GeV]; %s" % ac_ytitle, 100, 0., 200., 0., 2., "")

            hname = "ppt_%i" % i  # like the above pt histo, but zoomed in to 0-5 GeV
            histos[hname] = TProfile(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 5., 0., 2., "")
            histos["ac_"+hname] = TProfile("ac_"+hname, "; p_{T} [GeV]; %s" % ac_ytitle, 100, 0., 5., 0., 2., "")

            hname = "eta_%i" % i
            histos[hname] = TProfile(hname, "; #eta; %s" % ytitle, 120, -3.0, 3.0, 0., 2., "")
            histos["ac_"+hname] = TProfile("ac_"+hname, "; #eta; %s" % ac_ytitle, 120, -3.0, 3.0, 0., 2., "")

            hname = "phi_%i" % i
            histos[hname] = TProfile(hname, "; #phi; %s" % ytitle, 128, -3.2, 3.2, 0., 2., "")
            histos["ac_"+hname] = TProfile("ac_"+hname, "; #phi; %s" % ac_ytitle, 128, -3.2, 3.2, 0., 2., "")

            hname = "vz_%i" % i
            histos[hname] = TProfile(hname, "; vertex z_{0} [cm]; %s" % ytitle, 120, -30, 30, 0., 2., "")
            histos["ac_"+hname] = TProfile("ac_"+hname, "; vertex z_{0} [cm]; %s" % ac_ytitle, 120, -30, 30, 0., 2., "")

            hname = "charge_%i" % i
            histos[hname] = TProfile(hname, "; charge; %s" % ytitle, 5, -2.5, 2.5, 0., 2., "")
            histos["ac_"+hname] = TProfile("ac_"+hname, "; charge; %s" % ytitle, 5, -2.5, 2.5, 0., 2., "")

        colors1 = blkrgb[:3]
        colors2 = blkrgb[:1] + blkrgb[3:]
        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetFillStyle(0)
            v.SetMarkerStyle(24); v.SetMarkerSize(0.9)

            hname = v.GetName()
            i = int(hname[-1])
            hname = hname[:-2]
            if hname.startswith("ac_"):
                hname = hname[3:]
            if hname in ["pt", "ppt"]:
                v.SetMarkerColor(colors1[i % 3]); v.SetLineColor(colors1[i % 3])
            else:
                v.SetMarkerColor(colors2[i % 3]); v.SetLineColor(colors2[i % 3])

        # 2D: eta-phi
        for i in xrange(3):
            hname = "etaphi_%i" % i
            histos[hname] = TProfile2D(hname, "; #eta; #phi; %s" % ytitle, 60, -3, 3, 64, -3.2, 3.2, 0., 2., "")
            histos["ac_"+hname] = TProfile2D("ac_"+hname, "; #eta; #phi; %s" % ac_ytitle, 60, -3, 3, 64, -3.2, 3.2, 0., 2., "")
        return histos

    def projectCoverage(tree, histos, npatterns, nentries=1000000000, val=""):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("genParts_vz"           , 1)
        tree.SetBranchStatus("genParts_charge"       , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("AMTTRoads_bankIndex"   , 1)

        def fill(pt, eta, phi, vz, charge, accept, trigger):
            if accept:
                if abs(eta) < 2.2/3:
                    histos["pt_0"     + val].Fill(pt      , trigger)
                    histos["ppt_0"    + val].Fill(pt      , trigger)
                elif abs(eta) < 4.4/3:
                    histos["pt_1"     + val].Fill(pt      , trigger)
                    histos["ppt_1"    + val].Fill(pt      , trigger)
                elif abs(eta) < 6.6/3:
                    histos["pt_2"     + val].Fill(pt      , trigger)
                    histos["ppt_2"    + val].Fill(pt      , trigger)

                if pt >= 20:
                    histos["eta_0"    + val].Fill(eta     , trigger)
                    histos["phi_0"    + val].Fill(phi     , trigger)
                    histos["vz_0"     + val].Fill(vz      , trigger)
                    histos["charge_0" + val].Fill(charge  , trigger)
                    histos["etaphi_0" + val].Fill(eta, phi, trigger)
                elif pt >= 5:
                    histos["eta_1"    + val].Fill(eta     , trigger)
                    histos["phi_1"    + val].Fill(phi     , trigger)
                    histos["vz_1"     + val].Fill(vz      , trigger)
                    histos["charge_1" + val].Fill(charge  , trigger)
                    histos["etaphi_1" + val].Fill(eta, phi, trigger)
                elif pt >= 2:
                    histos["eta_2"    + val].Fill(eta     , trigger)
                    histos["phi_2"    + val].Fill(phi     , trigger)
                    histos["vz_2"     + val].Fill(vz      , trigger)
                    histos["charge_2" + val].Fill(charge  , trigger)
                    histos["etaphi_2" + val].Fill(eta, phi, trigger)

            if abs(eta) < 2.2/3:
                histos["ac_pt_0"     + val].Fill(pt     , trigger)
                histos["ac_ppt_0"    + val].Fill(pt     , trigger)
            elif abs(eta) < 4.4/3:
                histos["ac_pt_1"     + val].Fill(pt     , trigger)
                histos["ac_ppt_1"    + val].Fill(pt     , trigger)
            elif abs(eta) < 6.6/3:
                histos["ac_pt_2"     + val].Fill(pt     , trigger)
                histos["ac_ppt_2"    + val].Fill(pt     , trigger)

            if pt >= 20:
                histos["ac_eta_0"    + val].Fill(eta     , trigger)
                histos["ac_phi_0"    + val].Fill(phi     , trigger)
                histos["ac_vz_0"     + val].Fill(vz      , trigger)
                histos["ac_charge_0" + val].Fill(charge  , trigger)
                histos["ac_etaphi_0" + val].Fill(eta, phi, trigger)
            elif pt >= 5:
                histos["ac_eta_1"    + val].Fill(eta     , trigger)
                histos["ac_phi_1"    + val].Fill(phi     , trigger)
                histos["ac_vz_1"     + val].Fill(vz      , trigger)
                histos["ac_charge_1" + val].Fill(charge  , trigger)
                histos["ac_etaphi_1" + val].Fill(eta, phi, trigger)
            elif pt >= 2:
                histos["ac_eta_2"    + val].Fill(eta     , trigger)
                histos["ac_phi_2"    + val].Fill(phi     , trigger)
                histos["ac_vz_2"     + val].Fill(vz      , trigger)
                histos["ac_charge_2" + val].Fill(charge  , trigger)
                histos["ac_etaphi_2" + val].Fill(eta, phi, trigger)

        # Merged layers
        layermap = {
            5:0, 6:1, 7:2, 8:3, 9:4, 10:5,
            11:5, 12:4, 13:3, 14:2, 15:1,
            18:5, 19:4, 20:3, 21:2, 22:1,
        }

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            # Get track pt, eta, phi, vz, charge
            thePt, theEta, thePhi, theVz, theCharge = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0], ievt.genParts_vz[0], ievt.genParts_charge[0]

            if samples["tt27"]:
                if not (0 < theEta < 2.2/3 and pi/4 < thePhi < pi/2):
                    continue

            # Get trigger results
            trigger = False
            for bankIndex in ievt.AMTTRoads_bankIndex:
                if bankIndex < npatterns:
                    trigger = True

            # Loop over stub moduleIds
            moduleIds_by_mlayers = [[], [], [], [], [], []]  # 6 mlayers
            for moduleId in ievt.TTStubs_modId:
                lay = decodeLayer(moduleId)
                mlay = layermap[lay]
                if moduleId in ttrmap:
                    moduleIds_by_mlayers[mlay].append(moduleId)

            # At least one stub in every mlayer
            accept = True
            for moduleIds in moduleIds_by_mlayers:
                if not len(moduleIds):
                    accept = False
                    break

            # At least one stub combination is fully contained in at least one trigger tower
            if accept:
                accept = False
                for combination in itertools.product(*moduleIds_by_mlayers):
                    count_by_tt = {}
                    for moduleId in combination:
                        for tt in ttrmap[moduleId]:
                            count_by_tt[tt] = count_by_tt.get(tt, 0) + 1
                    for tt, count in count_by_tt.iteritems():
                        if samples["tt27"]:
                            if not (tt == 27):
                                continue
                        if count == 6:
                            accept = True
                            break
                    if accept:
                        break

            # Debug
            #print i_ievt
            #print "..", thePt, theEta, thePhi, theVz, theCharge, accept, trigger
            #for moduleId in ievt.TTStubs_modId:
            #    print "....", moduleId, ttrmap[moduleId]

            fill(thePt, theEta, thePhi, theVz, theCharge, accept, trigger)

        tree.SetBranchStatus("*", 1)
        return

    def drawCoverage(histos, ss, npatterns, hnlambda, imgdir):
        setting = parse_ss_to_setting(ss)
        for hvar in ["pt", "ppt", "eta", "phi", "vz", "charge"]:
            ymax = 1.2

            hname = "%s_%i" % (hvar,0)
            h = histos[hnlambda(hname)]
            h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax)
            h.Draw()

            # Reference lines for 0.4 and 1.0
            xmin, xmax = h.GetXaxis().GetXmin(), h.GetXaxis().GetXmax()
            line.DrawLine(xmin, 1.0, xmax, 1.0)
            line.DrawLine(xmin, 0.4, xmax, 0.4)
            for i in xrange(3):
                hname1 = "%s_%i" % (hvar,i)
                histos[hnlambda(hname1)].Draw("same")

            # Out of sample range
            tboxes = []
            if hvar == "pt" or hvar == "ppt":
                tboxes = [
                    TBox( xmin, 0,   2.0, ymax),
                    TBox(min(xmax,200.0), 0,  xmax, ymax),
                ]
            elif hvar == "eta":
                tboxes = [
                    TBox( xmin, 0,  -2.2, ymax),
                    TBox(  2.2, 0,  xmax, ymax),
                ]
            if samples["tt27"]:
                if hvar == "eta":
                    tboxes = [
                        TBox( xmin, 0,     0, ymax),
                        TBox(2.2/3, 0,  xmax, ymax),
                    ]
                elif hvar == "phi":
                    tboxes = [
                        TBox( xmin, 0,  pi/4, ymax),
                        TBox( pi/2, 0,  xmax, ymax),
                    ]
            for box in tboxes:
                box.SetFillStyle(3004); box.SetFillColor(kGray)
                box.Draw()

            # Legend
            moveLegend(0.48,0.22,0.94,0.34); legend.Clear()
            if hvar == "pt" or hvar == "ppt":
                writeme = [
                    "0.000 #leq |#eta| < 0.733",
                    "0.733 #leq |#eta| < 1.467",
                    "1.467 #leq |#eta| < 2.200",
                ]
            else:
                writeme = [
                    "20 #leq p_{T} < #infty  GeV",
                    "  5 #leq p_{T} < 20 GeV",
                    "  2 #leq p_{T} <   5 GeV",
                ]
            for i in xrange(3):
                hname1 = "%s_%i" % (hvar,i)
                legend.AddEntry(histos[hnlambda(hname1)], writeme[i], "lp")
            legend.Draw()

            latex.DrawLatex(0.6, 0.185, "%s [%.2fM bank]" % (setting, npatterns*1e-6))
            CMS_label()
            save(imgdir, "coverage_%s_%s" % (hnlambda(hname), ss), dot_root=True)

        # TH2F
        oldRightMargin = gPad.GetRightMargin()
        hvar = "etaphi"
        hname = "%s_%i" % (hvar,0)
        h = histos[hnlambda(hname)]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax)
        draw2D([h], stats=False)

        latex.DrawLatex(0.6, 0.235, "20 #leq p_{T} < #infty  GeV")
        latex.DrawLatex(0.6, 0.185, "%s [%.2fM bank]" % (setting, npatterns*1e-6))
        save(imgdir, "coverage_%s_%s" % (hnlambda(hname), ss), dot_root=True)

        gPad.SetRightMargin(oldRightMargin)
        donotdelete = [tboxes]
        return donotdelete


    # __________________________________________________________________________
    for ss, npatterns, coverage in settings:
        chain.Reset()

        infiles = listdir_fullpath(EOS + "/" + (results % ss) + "_" + DATE)
        print ss, len(infiles)
        for f in infiles:
            chain.Add(f)

        histos = bookCoverage()
        projectCoverage(chain, histos, npatterns, nentries=nentries)
        drawCoverage(histos, ss, npatterns, lambda x: x, imgdir)
        drawCoverage(histos, ss, npatterns, lambda x: "ac_"+x, imgdir)
        histos.clear()


# ______________________________________________________________________________
# Roads

if sections["roads"]:

    latex.SetTextSize(0.036)
    if not(samples["nu140"] or samples["tt27_nu140"] or samples["tttt140"] or samples["tt27_tttt140"]):
        print "ERROR!"
        exit

    def modify_binning(nbins, xmin, xmax):
        binsize = (xmax - xmin) / nbins
        return (nbins + 1, xmin - (binsize/2.), xmax + (binsize/2.))


    def bookRoads():
        histos = {}

        hname = "nroads_per_event"
        nbins, xmin, xmax = modify_binning(150, 0., 300.)
        histos[hname] = TH1F(hname, "; # roads per event"       , nbins, xmin, xmax)

        hname = "nsuperstrips_per_road"
        nbins, xmin, xmax = modify_binning(20, 0., 20.)
        histos[hname] = TH1F(hname, "; # superstrips per road"  , nbins, xmin, xmax)

        hname = "nstubs_per_superstrip"
        nbins, xmin, xmax = modify_binning(50, 0., 50.)
        histos[hname] = TH1F(hname, "; # stubs per superstrip"  , nbins, xmin, xmax)

        hname = "nstubs_per_road"
        nbins, xmin, xmax = modify_binning(50, 0., 50.)
        histos[hname] = TH1F(hname, "; # stubs per road"        , nbins, xmin, xmax)

        hname = "ncombinations_per_road"
        nbins, xmin, xmax = modify_binning(100, 0., 400.)
        histos[hname] = TH1F(hname, "; # combinations per road" , nbins, xmin, xmax)

        hname = "ncombinations_per_event"
        nbins, xmin, xmax = modify_binning(150, 0., 1200.)
        histos[hname] = TH1F(hname, "; # combinations per event", nbins, xmin, xmax)

        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)
            v.SetLineColor(col); v.SetFillColor(fcol)
        return histos

    def projectRoads(tree, histos, npatterns, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("AMTTRoads_bankIndex"        , 1)
        tree.SetBranchStatus("AMTTRoads_nsuperstrips"     , 1)
        tree.SetBranchStatus("AMTTRoads_stubSuperstripIds", 1)

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            nroads_per_event = 0
            ncombinations_per_event = 0

            for bankIndex, nsuperstrips, superstripIds in izip(ievt.AMTTRoads_bankIndex, ievt.AMTTRoads_nsuperstrips, ievt.AMTTRoads_stubSuperstripIds):

                if bankIndex < npatterns:
                    ssidmap = {}
                    for ssid in superstripIds:
                        ssidmap[ssid] = ssidmap.get(ssid, 0) + 1

                    nsuperstrips_per_road = len(ssidmap)
                    nroads_per_event += 1

                    assert(nsuperstrips_per_road == nsuperstrips)
                    histos["nsuperstrips_per_road"].Fill(nsuperstrips_per_road)

                    nstubs_per_road = 0
                    ncombinations_per_road = 1

                    for k, v in ssidmap.iteritems():
                        nstubs_per_superstrip = v
                        nstubs_per_road += v
                        ncombinations_per_road *= v

                        histos["nstubs_per_superstrip"].Fill(nstubs_per_superstrip)

                    ncombinations_per_event += ncombinations_per_road

                    assert(nstubs_per_road == superstripIds.size())
                    histos["nstubs_per_road"].Fill(nstubs_per_road)
                    histos["ncombinations_per_road"].Fill(ncombinations_per_road)

            assert(nroads_per_event <= ievt.AMTTRoads_nsuperstrips.size())
            histos["nroads_per_event"].Fill(nroads_per_event)
            histos["ncombinations_per_event"].Fill(ncombinations_per_event)

        tree.SetBranchStatus("*", 1)
        return

    def drawRoads(histos, ss, npatterns, imgdir, logy=False):
        setting = parse_ss_to_setting(ss)
        for hname, h in histos.iteritems():
            h.SetMinimum(0.5)
            draw([h], ytitle="", logy=logy)
            latex.DrawLatex(0.6, 0.185, "%s [%.2fM bank]" % (setting, npatterns*1e-6))
            save(imgdir, "roads_%s_%s" % (hname, ss), dot_root=True)


    # __________________________________________________________________________
    for ss, npatterns, coverage in settings:
        chain.Reset()

        infiles = listdir_fullpath(EOS + "/" + (results % ss) + "_" + DATE)
        print ss, len(infiles)
        for f in infiles:
            chain.Add(f)

        histos = bookRoads()
        projectRoads(chain, histos, npatterns, nentries=nentries/1000)
        drawRoads(histos, ss, npatterns, imgdir, logy=True)
        histos.clear()


# ______________________________________________________________________________
# Summary

if sections["summary"]:

    from collections import OrderedDict
    gStyle.SetNdivisions(520, "XYZ")

    latex.SetTextSize(0.05)
    latex2 = TLatex()
    latex2.SetNDC(0)
    latex2.SetTextFont(42)
    latex2.SetTextSize(0.04)

    def parse_lines(lines):
        k1, k2, x, y = [line.split() for line in lines]
        od = OrderedDict()
        for i in xrange(len(x)):
            if x[i] == "-1" or y[i] == "-1":
                continue
            if k1 == k2:
                k = "%s" % (k1[i])
            else:
                k = "(%s,%s)" % (k1[i], k2[i])
            od[k] = (float(x[i].replace(",","")), float(y[i].replace(",","")))
        return od

    def draw(plines, col=kBlack):
        gr = TGraph(len(plines))
        i = 0
        for k, v in plines.iteritems():
            print k, "-->", v
            gr.SetPoint(i, v[1], v[0])
            i += 1

        xmin, xmax = 10, 500
        ymin, ymax = 5e3, 5e6
        hframe = TH1F("hframe", "; <# combinations/tower/BX>; # patterns/tower", 100, xmin, xmax)
        hframe.SetStats(0); hframe.SetMinimum(ymin); hframe.SetMaximum(ymax)
        gr.SetMarkerStyle(21); gr.SetMarkerSize(1.2); gr.SetMarkerColor(col)

        hframe.Draw()
        latex.DrawLatex(0.12, 0.068, "10^{1}"); latex.DrawLatex(0.72, 0.068, "2#times10^{2}")
        gPad.SetLogx(1); gPad.SetLogy(1); gPad.SetGridx(1); gPad.SetGridy(1)
        gr.Draw("p")
        for k, v in plines.iteritems():
            latex2.DrawLatex(v[1], v[0], "  #color[%i]{%s}" % (col,k))
        CMS_label()
        #save(imgdir, "summary")
        #gPad.SetLogx(0); gPad.SetLogy(0); gPad.SetGridx(0); gPad.SetGridy(0)

        donotdelete = [hframe, gr]
        return donotdelete

    def draw2(plines, plines2, col=kBlack, col2=kBlack):
        gr = TGraph(len(plines))
        i = 0
        for k, v in plines.iteritems():
            print k, "-->", v
            gr.SetPoint(i, v[1], v[0])
            i += 1

        gr2 = TGraph(len(plines2))
        i = 0
        for k, v in plines2.iteritems():
            print k, "-->", v
            gr2.SetPoint(i, v[1], v[0])
            i += 1

        xmin, xmax = 10, 500
        ymin, ymax = 5e3, 5e6
        hframe = TH1F("hframe", "; <# combinations/tower/BX>; # patterns/tower", 100, xmin, xmax)
        hframe.SetStats(0); hframe.SetMinimum(ymin); hframe.SetMaximum(ymax)
        gr.SetMarkerStyle(21); gr.SetMarkerSize(1.2); gr.SetMarkerColor(col)
        gr2.SetMarkerStyle(21); gr2.SetMarkerSize(1.2); gr2.SetMarkerColor(col2)

        hframe.Draw()
        latex.DrawLatex(0.12, 0.068, "10^{1}"); latex.DrawLatex(0.72, 0.068, "2#times10^{2}")
        gPad.SetLogx(1); gPad.SetLogy(1); gPad.SetGridx(1); gPad.SetGridy(1)
        gr.Draw("p")
        for k, v in plines.iteritems():
            latex2.DrawLatex(v[1], v[0], "  #color[%i]{%s}" % (col,k))
        gr2.Draw("p")
        for k, v in plines2.iteritems():
            latex2.DrawLatex(v[1], v[0], "  #color[%i]{%s}" % (col2,k))
        CMS_label()
        #save(imgdir, "summary")
        #gPad.SetLogx(0); gPad.SetLogy(0); gPad.SetGridx(0); gPad.SetGridy(0)

        donotdelete = [hframe, gr, gr2]
        return donotdelete


    lines_fixed = [
        "ss32	ss64	ss128	ss256	ss512	ss1024",
        "ss32	ss64	ss128	ss256	ss512	ss1024",
        "-1	5,002,400	1,314,880	344,890	92,530	27,940",
        "-1.0	18.7	38.3	75.6	147.8	397.9",
    ]
    plines_fixed = parse_lines(lines_fixed)
    #d = draw(plines_fixed, col=blkrgb[3])

    lines_projective = [
        "600x0	400x0	200x0	200x1	100x2	20x10",
        "600x0	400x0	200x0	200x1	100x2	20x10",
        "2,033,820	486,640	70,130	295,470	205,440	198,990",
        "11.7	17.5	47.2	33.6	42.6	79.2",
    ]
    plines_projective = parse_lines(lines_projective)
    #d = draw(plines_projective, col=blkrgb[2])

    lines_rational = [
        "0.5	0.7	0.7	0.7	1	1	1	2	2	2",
        "1	1	2	4	1	2	4	1	2	4",
        "275,290	107,500	590,260	-1	44,810	252,920	1,180,040	9,730	52,720	270,890",
        "22.5	33.4	18.4	-1.0	68.5	33.2	16.9	194.6	90.9	46.1",
    ]
    plines_rational = parse_lines(lines_rational)
    #d = draw(plines_rational, col=blkrgb[1])

    lines_rational2 = [
        "0.5	0.7	1	2",
        "1	1	1	1",
        "275,290	107,500	44,810	9,730",
        "22.5	33.4	68.5	194.6",
    ]
    plines_rational2 = parse_lines(lines_rational2)
    d = draw2(plines_rational2, plines_fixed, col=blkrgb[1], col2=blkrgb[3])

