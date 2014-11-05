#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
import itertools
import os

# ______________________________________________________________________________
# Configurations

samples = {}
samples["nu140"     ] = False
samples["tt27"      ] = True
samples["tt27_nu140"] = False

sections = {}
sections["coverage"     ] = True
sections["roads"        ] = False

drawerInit = DrawerInit()
gStyle.SetNumberContours(100)
gStyle.SetPalette(55)  # rainbow color map


EOS = "/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_results/"
RES = "SingleMuPlusMinus_sp16_%s"
#RES = "SingleElePlusMinus_sp16_%s"
#RES = "SinglePionPlusMinus_sp16_%s"
#RES = "SingleKaonPlusMinus_sp16_%s"

DATE = "20140926"

settings = [
    ("ss256"  ,  87631104),
]

settings_tt27 = [
    ("ss1024" ,   479095),
    ("ss512"  ,  1382352),
    ("ss256"  ,  4109588),
    ("ss128"  , 11612095),
    ("ss64"   , 29093132),
    ("ss32"   , 64981077),
]

col  = TColor.GetColor("#1f78b4")  # Paired
fcol = TColor.GetColor("#a6cee3")
imgdir = "figures_amsim/"
#imgdir = "figures_amsim/ele/"
#imgdir = "figures_amsim/pion/"
#imgdir = "figures_amsim/kaon/"

if samples["tt27"]:
    RES = "SingleMuPlusMinus_sp16_%s_tt27"
    settings = settings_tt27
    imgdir = "figures_amsim_tt27/"

if samples["nu140"]:
    EOS = EOS.replace("_results","_results_PU140bx25")
    RES = "Neutrino_sp16_%s"

    col = TColor.GetColor("#e31a1c")  # Paired
    fcol = TColor.GetColor("#fb9a99")
    imgdir = "figures_amsim_nu140/"

if samples["tt27_nu140"]:
    EOS = EOS.replace("_results","_results_PU140bx25")
    RES = "Neutrino_sp16_%s_tt27"
    settings = settings_tt27

    col = TColor.GetColor("#e31a1c")  # Paired
    fcol = TColor.GetColor("#fb9a99")
    imgdir = "figures_amsim_tt27_nu140/"


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
    return [os.path.join(d, f) for f in os.listdir(d)]


# ______________________________________________________________________________
# Coverage

if sections["coverage"]:

    latex.SetTextSize(0.036)
    if samples["nu140"] or samples["tt27_nu140"]:
        print "ERROR!"
        exit

    def bookCoverage():
        histos = {}

        # 1D: pt, pt (zoom), eta, phi, vz
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

    def projectCoverage(tree, histos, nentries=1000000000, val=""):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("genParts_vz"           , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("AMTTRoads_nHitLayers"  , 1)

        def fill(pt, eta, phi, vz, accept, trigger):
            if accept:
                if abs(eta) < 0.8:
                    histos["pt_0"     + val].Fill(pt , trigger)
                    histos["ppt_0"    + val].Fill(pt , trigger)
                elif abs(eta) < 1.6:
                    histos["pt_1"     + val].Fill(pt , trigger)
                    histos["ppt_1"    + val].Fill(pt , trigger)
                elif abs(eta) < 2.2:
                    histos["pt_2"     + val].Fill(pt , trigger)
                    histos["ppt_2"    + val].Fill(pt , trigger)

                if pt >= 20:
                    histos["eta_0"    + val].Fill(eta, trigger)
                    histos["phi_0"    + val].Fill(phi, trigger)
                    histos["vz_0"     + val].Fill(vz , trigger)
                    histos["etaphi_0" + val].Fill(eta, phi, trigger)
                elif pt >= 5:
                    histos["eta_1"    + val].Fill(eta, trigger)
                    histos["phi_1"    + val].Fill(phi, trigger)
                    histos["vz_1"     + val].Fill(vz , trigger)
                    histos["etaphi_1" + val].Fill(eta, phi, trigger)
                elif pt >= 2:
                    histos["eta_2"    + val].Fill(eta, trigger)
                    histos["phi_2"    + val].Fill(phi, trigger)
                    histos["vz_2"     + val].Fill(vz , trigger)
                    histos["etaphi_2" + val].Fill(eta, phi, trigger)

            if abs(eta) < 0.8:
                histos["ac_pt_0"     + val].Fill(pt , trigger)
                histos["ac_ppt_0"    + val].Fill(pt , trigger)
            elif abs(eta) < 1.6:
                histos["ac_pt_1"     + val].Fill(pt , trigger)
                histos["ac_ppt_1"    + val].Fill(pt , trigger)
            elif abs(eta) < 2.2:
                histos["ac_pt_2"     + val].Fill(pt , trigger)
                histos["ac_ppt_2"    + val].Fill(pt , trigger)

            if pt >= 20:
                histos["ac_eta_0"    + val].Fill(eta, trigger)
                histos["ac_phi_0"    + val].Fill(phi, trigger)
                histos["ac_vz_0"     + val].Fill(vz , trigger)
                histos["ac_etaphi_0" + val].Fill(eta, phi, trigger)
            elif pt >= 5:
                histos["ac_eta_1"    + val].Fill(eta, trigger)
                histos["ac_phi_1"    + val].Fill(phi, trigger)
                histos["ac_vz_1"     + val].Fill(vz , trigger)
                histos["ac_etaphi_1" + val].Fill(eta, phi, trigger)
            elif pt >= 2:
                histos["ac_eta_2"    + val].Fill(eta, trigger)
                histos["ac_phi_2"    + val].Fill(phi, trigger)
                histos["ac_vz_2"     + val].Fill(vz , trigger)
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

            # Get track pt, eta, phi, vz
            thePt, theEta, thePhi, theVz = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0], ievt.genParts_vz[0]

            # Get trigger results
            trigger = (ievt.AMTTRoads_nHitLayers.size() > 0)

            # Loop over stub moduleIds
            moduleIds_by_mlayers = [[], [], [], [], [], []]  # 6 mlayers
            for moduleId in ievt.TTStubs_modId:
                lay = decodeLayer(moduleId)
                mlay = layermap[lay]
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
                            # Idiom for incrementing
                            # See: http://www.gis.sdsu.edu/~gawron/compling/course_core/assignments/dictionary_discussion.htm
                            count_by_tt[tt] = count_by_tt.get(tt, 0) + 1
                    for tt, count in count_by_tt.iteritems():
                        if count == 6:
                            accept = True
                            break
                    if accept:
                        break

            fill(thePt, theEta, thePhi, theVz, accept, trigger)

        tree.SetBranchStatus("*", 1)
        return

    def drawCoverage(histos, setting, banksize, hnlambda, imgdir):
        for hvar in ["pt", "ppt", "eta", "phi", "vz"]:
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
                        TBox( xmin, 0,  -0.2, ymax),
                        TBox(  0.8, 0,  xmax, ymax),
                    ]
                elif hvar == "phi":
                    tboxes = [
                        TBox( xmin, 0,   0.6, ymax),
                        TBox(  1.7, 0,  xmax, ymax),
                    ]
            for box in tboxes:
                box.SetFillStyle(3004); box.SetFillColor(kGray)
                box.Draw()

            # Legend
            moveLegend(0.48,0.22,0.94,0.34); legend.Clear()
            if hvar == "pt" or hvar == "ppt":
                writeme = [
                    "0.0 #leq |#eta| < 0.8",
                    "0.8 #leq |#eta| < 1.6",
                    "1.6 #leq |#eta| < 2.2",
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

            latex.DrawLatex(0.6, 0.185, "%s [%.1fM bank]" % (setting, banksize))
            CMS_label()
            save(imgdir, "coverage_%s_%s" % (hnlambda(hname), setting), dot_root=True)

        # TH2F
        oldRightMargin = gPad.GetRightMargin()
        hvar = "etaphi"
        hname = "%s_%i" % (hvar,0)
        h = histos[hnlambda(hname)]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax)
        draw2D([h], stats=False)

        latex.DrawLatex(0.6, 0.235, "20 #leq p_{T} < #infty  GeV")
        latex.DrawLatex(0.6, 0.185, "%s [%.1fM bank]" % (setting, banksize))
        save(imgdir, "coverage_%s_%s" % (hnlambda(hname), setting), dot_root=True)

        gPad.SetRightMargin(oldRightMargin)
        donotdelete = [tboxes]
        return donotdelete


    # __________________________________________________________________________
    for sett, sett1 in settings[:]:
        chain.Reset()

        result = (RES % sett) + ("_%s" % DATE)
        infiles = listdir_fullpath(EOS+"/"+result)
        print sett, len(infiles)
        for f in infiles:
            chain.Add(f)

        histos = bookCoverage()
        projectCoverage(chain, histos, nentries=nentries)
        setting = sett.replace("_", " ")
        banksize = 1e-6*sett1
        drawCoverage(histos, setting, banksize, lambda x: x, imgdir)
        drawCoverage(histos, setting, banksize, lambda x: "ac_"+x, imgdir)
        histos.clear()


# ______________________________________________________________________________
# Roads

if sections["roads"]:

    latex.SetTextSize(0.036)
    if not (samples["nu140"] or samples["tt27_nu140"]):
        print "ERROR!"
        exit

    def bookRoads():
        histos = {}

        hname = "nroads_per_event"
        histos[hname] = TH1F(hname, "; # roads per event"       , 50, 0, 200)

        hname = "nsuperstrips_per_road"
        histos[hname] = TH1F(hname, "; # superstrips per road"  , 20, 0,  20)

        hname = "nstubs_per_superstrip"
        histos[hname] = TH1F(hname, "; # stubs per superstrip"  , 30, 0,  30)

        hname = "nstubs_per_road"
        histos[hname] = TH1F(hname, "; # stubs per road"        , 40, 0,  40)

        hname = "ncombinations_per_road"
        histos[hname] = TH1F(hname, "; # combinations per road" , 60, 0, 300)

        hname = "ncombinations_per_event"
        histos[hname] = TH1F(hname, "; # combinations per event", 80, 0, 800)

        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)
            v.SetLineColor(col); v.SetFillColor(fcol)
        return histos

    def projectRoads(tree, histos, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("AMTTRoads_nHitLayers"      , 1)
        tree.SetBranchStatus("AMTTRoads_hitSuperstripIds", 1)

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            nroads_per_event = 0
            ncombinations_per_event = 0

            for nHitLayers, superstripIds in izip(ievt.AMTTRoads_nHitLayers, ievt.AMTTRoads_hitSuperstripIds):
                ssmap = {}
                for ss in superstripIds:
                    if ss not in ssmap:
                        ssmap[ss] = 1
                    else:
                        ssmap[ss] += 1

                nsuperstrips_per_road = len(ssmap)
                nroads_per_event += 1

                assert(nsuperstrips_per_road == nHitLayers)
                histos["nsuperstrips_per_road"].Fill(nsuperstrips_per_road)

                nstubs_per_road = 0
                ncombinations_per_road = 1.

                for k, v in ssmap.iteritems():
                    nstubs_per_superstrip = v
                    nstubs_per_road += v
                    ncombinations_per_road *= v

                    histos["nstubs_per_superstrip"].Fill(nstubs_per_superstrip)

                ncombinations_per_event += ncombinations_per_road

                assert(nstubs_per_road == superstripIds.size())
                histos["nstubs_per_road"].Fill(nstubs_per_road)
                histos["ncombinations_per_road"].Fill(ncombinations_per_road)

            assert(nroads_per_event == ievt.AMTTRoads_nHitLayers.size())
            histos["nroads_per_event"].Fill(nroads_per_event)
            histos["ncombinations_per_event"].Fill(ncombinations_per_event)

        tree.SetBranchStatus("*", 1)
        return

    def drawRoads(histos, setting, banksize, imgdir, logy=False):
        for hname, h in histos.iteritems():
            draw([h], ytitle="", logy=logy)
            latex.DrawLatex(0.6, 0.185, "%s [%.1fM bank]" % (setting, banksize))
            save(imgdir, "roads_%s_%s" % (hname, setting), dot_root=True)


    # __________________________________________________________________________
    for sett, sett1 in settings[:]:
        chain.Reset()

        result = (RES % sett) + ("_%s" % DATE)
        infiles = listdir_fullpath(EOS+"/"+result)
        print sett, len(infiles)
        for f in infiles:
            chain.Add(f)

        histos = bookRoads()
        projectRoads(chain, histos, nentries=nentries/1000)
        setting = sett.replace("_", " ")
        banksize = 1e-6*sett1
        drawRoads(histos, setting, banksize, imgdir, logy=True)
        histos.clear()
