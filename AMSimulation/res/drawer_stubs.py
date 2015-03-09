#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
from array import array
from math import sqrt, pi, sin, cos, tan, sinh

# ______________________________________________________________________________
# Configurations

sections = {}
sections["residual"     ] = False
sections["rcoord"       ] = False
sections["interval"     ] = False
sections["multiplicity" ] = True
sections["multiplicity2"] = False


drawerInit = DrawerInit()

infiles = ["/uscms_data/d2/jiafu/L1TrackTrigger/CRAB_amsim_luciano/stubs_barrel_2M.2.root"]
infilestxt = "/uscms_data/d2/jiafu/L1TrackTrigger/CRAB_amsim_luciano/input_bank_barrel_20141105.txt"

col  = TColor.GetColor("#1f78b4")  # Paired
fcol = TColor.GetColor("#a6cee3")
imgdir = "figures_stubs/"


# Number of events
nentries = 2000000
#nentries = 2000

chain = TChain("ntupler/tree", "")
for f in infiles:
    chain.Add(f)
tree = chain

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)


# ______________________________________________________________________________
# Load

ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)


# ______________________________________________________________________________
# Global

latex2 = TLatex()
latex2.SetNDC()
latex2.SetTextFont(42)
latex2.SetTextSize(0.034)

line2 = TLine()
line2.SetLineColor(kGray+3)
line2.SetLineStyle(1)
line2.SetLineWidth(2)


# ______________________________________________________________________________
if sections["residual"]:

    def doit(x, imgdir=None, logy=False):
        histos = book(x)
        project(tree, histos, nentries=nentries, drawOverflow=False, drawUnderflow=False)
        draw(histos, logy=logy)
        save(imgdir, x[0].name)
        return histos

    # Exactly 6 stubs
    cut = "(@TTStubs_r.size()==6)"

    # Barrel
    phimaxs = [0.006, 0.008, 0.010, 0.014, 0.018, 0.024]
    zmaxs = [0.6, 0.6, 0.6, 6, 6, 6]
    for i in xrange(5,11):
        layer = "b%i" % i
        ii = i-5

        var = "TTStubs_phi[%i] - (genParts_phi[0] - 0.3*3.8*TTStubs_r[%i]*1e-2 * (genParts_charge[0]/genParts_pt[0]) / 2.0)" % (ii,ii)
        xtitle = "stub #phi_{%s} - ideal #phi_{%s}" % (layer,layer)
        p0 = struct("phiresidual_%s" % layer, var, cut, xtitle, 240, -phimaxs[ii], phimaxs[ii], col, fcol)
        histos = doit([p0], imgdir)
        print histos[0].h.Integral(), histos[0].h.GetRMS()

        var = "TTStubs_z[%i] - (genParts_vz[0] + TTStubs_r[%i] * sinh(genParts_eta[0]))" % (ii,ii)  # cot(theta) = sinh(eta) = pz/pt
        xtitle = "stub z_{%s} - ideal z_{%s} [cm]" % (layer,layer)
        p0 = struct("zresidual_%s" % layer, var, cut, xtitle, 240, -zmaxs[ii], zmaxs[ii], col, fcol)
        histos = doit([p0], imgdir)
        print histos[0].h.Integral(), histos[0].h.GetRMS()


# ______________________________________________________________________________
if sections["rcoord"]:

    def doit(x, imgdir=None, logy=False, ymax=2000000./4/2):
        histos = book(x)
        project(tree, histos, nentries=nentries, drawOverflow=False, drawUnderflow=False)
        histos[0].h.SetMaximum(ymax / 1.4)
        draw(histos, logy=logy)
        save(imgdir, x[0].name)
        return histos

    # Exactly 6 stubs
    cut = "(@TTStubs_r.size()==6)"

    # Barrel
    rmeans = b_rcoord_cmssw

    for i in xrange(5,11):
        layer = "b%i" % i
        ii = i-5
        r = rmeans[ii]

        var = "TTStubs_r"
        xtitle = "stub r_{%s} [cm]" % layer
        cut2 = "(%i<=TTStubs_modId && TTStubs_modId<%i)" % (i*10000, (i+1)*10000)
        p0 = struct("rcoord_%s" % layer, var, cut+"*"+cut2, xtitle, 250, r-2.5, r+2.5, col, fcol)
        histos = doit([p0], imgdir)
        print histos[0].h.Integral(), histos[0].h.GetRMS()


# ______________________________________________________________________________
if sections["interval"]:

    def bookLayer(tt):
        histos = {}
        for i in xrange(5,23):
            if i == 16 or i == 17:  continue
            if i < 11:
                layer = "b%i" % i
            else:
                layer = "e%i" % i
            h = TH1F("phiinterval_tt%i_%s" % (tt,layer), "; stub #phi_{%s}; Entries" % layer, 640*2, -3.2, 3.2)
            histos[h.GetName()] = h
            h = TH1F("etainterval_tt%i_%s" % (tt,layer), "; stub #eta_{%s}; Entries" % layer, 600*2, -3.0, 3.0)
            histos[h.GetName()] = h
            h = TH1F("xinterval_tt%i_%s" % (tt,layer), "; stub r#phi_{%s} [cm]; Entries" % layer, 600*2, -300.0, 300.0)
            histos[h.GetName()] = h
            h = TH1F("zinterval_tt%i_%s" % (tt,layer), "; stub z_{%s} [cm]; Entries" % layer, 600*2, -300.0, 300.0)
            histos[h.GetName()] = h
        return histos

    def projectLayer(tree, histos, tt, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("TTStubs_r"      , 1)
        tree.SetBranchStatus("TTStubs_phi"    , 1)
        tree.SetBranchStatus("TTStubs_eta"    , 1)
        tree.SetBranchStatus("TTStubs_z"      , 1)
        tree.SetBranchStatus("TTStubs_modId"  , 1)

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            for r, phi, eta, z, moduleId in izip(ievt.TTStubs_r, ievt.TTStubs_phi, ievt.TTStubs_eta, ievt.TTStubs_z, ievt.TTStubs_modId):
                if moduleId in ttmap[tt]:
                    lay = decodeLayer(moduleId)
                    if lay < 11:
                        layer = "b%i" % lay
                    else:
                        layer = "e%i" % lay
                    histos["phiinterval_tt%i_%s" % (tt,layer)].Fill(phi)
                    histos["etainterval_tt%i_%s" % (tt,layer)].Fill(eta)
                    histos["xinterval_tt%i_%s" % (tt,layer)].Fill(r*phi)
                    histos["zinterval_tt%i_%s" % (tt,layer)].Fill(z)

        tree.SetBranchStatus("*", 1)
        return

    def find_interval(h):
        nbins = h.GetNbinsX()
        xmin, xmax = 0, nbins+2
        for i in xrange(nbins+2):
            if h.GetBinContent(i) > 0:
                xmin = h.GetBinLowEdge(i)
                break
        for i in xrange(nbins+2):
            if h.GetBinContent(nbins-1-i) > 0:
                xmax = h.GetBinLowEdge(nbins-i)
                break
        return xmin, xmax

    def drawLayer(histos, imgdir, tt):
        for i in xrange(5,23):
            if i == 16 or i == 17:  continue
            if i < 11:
                layer = "b%i" % i
            else:
                layer = "e%i" % i

            # intervals
            for hname, hdesc in [("phiinterval_tt%i_%s", "%s #phi=[%.3f,%.3f]" ),
                                 ("etainterval_tt%i_%s", "%s #eta=[%.3f,%.3f]" ),
                                 ("xinterval_tt%i_%s"  , "%s r#phi=[%.3f,%.3f]"),
                                 ("zinterval_tt%i_%s"  , "%s z=[%.3f,%.3f]"    )]:
                h = histos[hname % (tt,layer)]
                h.SetStats(0); h.SetLineWidth(2); h.SetLineColor(col); h.SetFillColor(fcol)
                if h.GetEntries() == 0:  continue
                xmin, xmax = find_interval(h)
                h.Rebin(10); h.SetMaximum(h.GetMaximum()*1.3)
                h.Draw("hist")

                moveLegend(0.56,0.88,0.94,0.94); legend.Clear()
                legend.AddEntry(h, hdesc % (layer,xmin,xmax), "f")
                legend.Draw()
                CMS_label()
                save(imgdir, hname % (tt,layer))

        donotdelete = []
        return donotdelete

    # Barrel tt27
    for tt in [27]:
        histos = bookLayer(tt)
        projectLayer(tree, histos, tt, nentries=nentries)
        d = drawLayer(histos, imgdir, tt)


# ______________________________________________________________________________
if sections["multiplicity"]:

    """Before stub cleaning"""

    from ROOT import TFileCollection

    chain2 = TChain("ntupler/tree", "")
    fc = TFileCollection("fileinfolist", "", infilestxt)
    chain2.AddFileInfoList(fc.GetList())
    tree2 = chain2

    in_quantiles = array('d', [0.0015, 0.025, 0.16, 0.5, 0.84, 0.975, 0.9985])
    quantiles = array('d', [0., 0., 0., 0., 0., 0., 0.])

    phimaxs = [0.006, 0.008, 0.010, 0.014, 0.018, 0.024]
    #xmaxs = [0.14, 0.30, 0.60, 1.00, 1.60, 2.60]
    xmaxs = [0.6, 0.6, 0.6, 1.2, 1.2, 1.2]
    zmaxs = [0.6, 0.6, 0.6, 6, 6, 6]
    phicuts = [0.004, 0.004, 0.005, 0.007, 0.009, 0.012]
    #xcuts = [0.030, 0.070, 0.130, 0.270, 0.540, 0.810]
    #zcuts = [0.150, 0.300, 0.300, 5.000, 5.000, 5.000]
    xcuts = [0.040, 0.070, 0.130, 0.240, 0.450, 0.780]
    zcuts = [0.300, 0.400, 0.500, 3.000, 3.500, 4.000]

    def bookLayer():
        histos = {}
        for i in xrange(5,11):
            layer = "b%i" % i
            ii = i-5

            h = TH1F("nstubs_%s" % layer, "; # stubs_{%s}; Entries" % layer, 10, 0, 10)
            histos[h.GetName()] = h
            h = TH1F("phiresidual_%s" % layer, "; stub #phi_{%s} - ideal #phi_{%s}; Entries" % (layer, layer), 240, -phimaxs[ii], phimaxs[ii])
            histos[h.GetName()] = h
            h = TH1F("xresidual_%s" % layer, "; stub r#phi_{%s} - ideal r#phi_{%s} [cm]; Entries" % (layer, layer), 240, -xmaxs[ii], xmaxs[ii])
            histos[h.GetName()] = h
            h = TH1F("zresidual_%s" % layer, "; stub z_{%s} - ideal z_{%s} [cm]; Entries" % (layer, layer), 240, -zmaxs[ii], zmaxs[ii])
            histos[h.GetName()] = h

            h = TH1F("phiresidual_clean_%s" % layer, "; stub #phi_{%s} - ideal #phi_{%s}; Entries" % (layer, layer), 240, -phimaxs[ii], phimaxs[ii])
            histos[h.GetName()] = h
            h = TH1F("xresidual_clean_%s" % layer, "; stub r#phi_{%s} - ideal r#phi_{%s} [cm]; Entries" % (layer, layer), 240, -xmaxs[ii], xmaxs[ii])
            histos[h.GetName()] = h
            h = TH1F("zresidual_clean_%s" % layer, "; stub z_{%s} - ideal z_{%s} [cm]; Entries" % (layer, layer), 240, -zmaxs[ii], zmaxs[ii])
            histos[h.GetName()] = h

            # Use mean r
            h = TH1F("phiresidual_rmean_%s" % layer, "; stub #phi_{%s} - ideal #phi_{%s} {mean r}; Entries" % (layer, layer), 240, -phimaxs[ii], phimaxs[ii])
            histos[h.GetName()] = h

            h = TH1F("phiresidual_clean_rmean_%s" % layer, "; stub #phi_{%s} - ideal #phi_{%s} {mean r}; Entries" % (layer, layer), 240, -phimaxs[ii], phimaxs[ii])
            histos[h.GetName()] = h
        return histos

    def projectLayer(tree, tree2, histos, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"    , 1)
        tree.SetBranchStatus("genParts_eta"   , 1)
        tree.SetBranchStatus("genParts_phi"   , 1)
        tree.SetBranchStatus("genParts_vz"    , 1)
        tree.SetBranchStatus("genParts_charge", 1)
        tree.SetBranchStatus("TTStubs_r"      , 1)
        tree.SetBranchStatus("TTStubs_phi"    , 1)
        tree.SetBranchStatus("TTStubs_z"      , 1)
        tree.SetBranchStatus("TTStubs_modId"  , 1)

        tree2.SetBranchStatus("*", 0)
        tree2.SetBranchStatus("TTStubs_r"      , 1)
        tree2.SetBranchStatus("TTStubs_phi"    , 1)
        tree2.SetBranchStatus("TTStubs_z"      , 1)
        tree2.SetBranchStatus("TTStubs_modId"  , 1)

        ievt, ievt2 = tree, tree2
        for i_ievt in xrange(nentries):
            tree.GetEntry(i_ievt)
            tree2.GetEntry(i_ievt)

            simPt, simEta, simPhi, simVZ, simCharge = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0], ievt.genParts_vz[0], ievt.genParts_charge[0]

            # Exactly 6 stubs
            if len(ievt.TTStubs_r) == 6:
                for r, phi, z, moduleId in izip(ievt.TTStubs_r, ievt.TTStubs_phi, ievt.TTStubs_z, ievt.TTStubs_modId):
                    lay = decodeLayer(moduleId)
                    layer = "b%i" % lay
                    if lay > 10:  print "ERROR! lay:", lay
                    ilay = lay - 5

                    deltaPhi = - 0.3*3.8*r*1e-2 * (simCharge/simPt) / 2.0
                    idealPhi = simPhi + deltaPhi
                    idealZ = r*sinh(simEta)  # cot(theta) = sinh(eta) = pz/pt

                    histos["phiresidual_clean_%s" % layer].Fill(phi - idealPhi)
                    histos["xresidual_clean_%s" % layer].Fill(r * (phi - idealPhi))
                    histos["zresidual_clean_%s" % layer].Fill(z - (idealZ + simVZ))

                    rmean = b_rcoord_cmssw[ilay]
                    deltaPhi_rmean = deltaPhi * rmean / r
                    idealPhi_rmean = simPhi + deltaPhi_rmean
                    histos["phiresidual_clean_rmean_%s" % layer].Fill(phi - idealPhi_rmean)

                assert(len(ievt2.TTStubs_r) >= len(ievt.TTStubs_r))
                nstubs = [0, 0, 0, 0, 0, 0]
                for r, phi, z, moduleId in izip(ievt2.TTStubs_r, ievt2.TTStubs_phi, ievt2.TTStubs_z, ievt2.TTStubs_modId):
                    lay = decodeLayer(moduleId)
                    layer = "b%i" % lay
                    if lay > 10:  continue
                    ilay = lay - 5

                    deltaPhi = - 0.3*3.8*r*1e-2 * (simCharge/simPt) / 2.0
                    idealPhi = simPhi + deltaPhi
                    idealZ = r*sinh(simEta)  # cot(theta) = sinh(eta) = pz/pt

                    histos["phiresidual_%s" % layer].Fill(phi - idealPhi)
                    histos["xresidual_%s" % layer].Fill(r * (phi - idealPhi))
                    histos["zresidual_%s" % layer].Fill(z - (idealZ + simVZ))

                    rmean = b_rcoord_cmssw[ilay]
                    deltaPhi_rmean = deltaPhi * rmean / r
                    idealPhi_rmean = simPhi + deltaPhi_rmean
                    histos["phiresidual_rmean_%s" % layer].Fill(phi - idealPhi_rmean)

                    nstubs[ilay] += 1

                for i in xrange(5,11):
                    layer = "b%i" % i
                    ii = i-5
                    histos["nstubs_%s" % layer].Fill(nstubs[ii])

        tree.SetBranchStatus("*", 1)
        tree2.SetBranchStatus("*", 1)
        return

    def drawLayer(histos, imgdir):
        printme = []

        for i in xrange(5,11):
            layer = "b%i" % i
            ii = i-5

            # nstubs
            h0 = histos["nstubs_%s" % layer]
            h0.SetLineWidth(2); h0.SetLineColor(kBlack); h0.SetFillColor(kGray+1)
            h0.SetMaximum(h0.GetMaximum()*10)
            h0.Draw("hist")
            gPad.SetLogy(1)
            CMS_label()
            save(imgdir, "multiplicity_"+h0.GetName())
            gPad.SetLogy(0)

            # residuals
            for hname in ["phiresidual_%s",
                          "phiresidual_rmean_%s",
                          "xresidual_%s",
                          "zresidual_%s",
                          ]:
                h0 = histos[hname % layer]
                h0.SetLineWidth(2); h0.SetLineColor(kBlack); h0.SetFillColor(kGray+1)
                ymax = h0.GetMaximum()
                h0.SetMinimum(0); h0.SetMaximum(ymax*1.4)
                h0.SetStats(0); h0.Draw("hist")

                h1 = histos[(hname.replace("residual_", "residual_clean_")) % layer]
                h1.SetLineWidth(2); h1.SetLineColor(col); h1.SetFillColor(fcol)
                h1.Draw("same hist")

                moveLegend(0.56,0.74,0.92,0.94); legend.Clear()
                legend.AddEntry(h0, "default #splitline{#mu=%.3e}{#sigma=%.3e}" % (h0.GetMean(),h0.GetRMS()), "f")
                legend.AddEntry(h1, "cleaned #splitline{#mu=%.3e}{#sigma=%.3e}" % (h1.GetMean(),h1.GetRMS()), "f")
                legend.Draw()

                h1.GetQuantiles(len(quantiles), quantiles, in_quantiles)
                printme.append('{} {} {}'.format(h1.GetName(), h1.GetMean()+2.0*h1.GetRMS(), quantiles[5]))

                if "phiresidual" in hname:
                    line2.DrawLine(phicuts[ii], 0, phicuts[ii], ymax)
                    line2.DrawLine(-phicuts[ii], 0, -phicuts[ii], ymax)

                    eff = h1.Integral(h1.FindFixBin(-phicuts[ii]), h1.FindFixBin(phicuts[ii])) / h1.Integral(0,999999) * 100.
                    latex2.DrawLatex(0.2, 0.88, "cut:  #Delta < %.3f cm" % phicuts[ii])
                    latex2.DrawLatex(0.2, 0.84, "efficiency: %.1f%%" % eff)
                    printme.append('  {} {}'.format(phicuts[ii], eff))

                elif "xresidual" in hname:
                    line2.DrawLine(xcuts[ii], 0, xcuts[ii], ymax)
                    line2.DrawLine(-xcuts[ii], 0, -xcuts[ii], ymax)

                    eff = h1.Integral(h1.FindFixBin(-xcuts[ii]), h1.FindFixBin(xcuts[ii])) / h1.Integral(0,999999) * 100.
                    latex2.DrawLatex(0.2, 0.88, "cut:  #Delta < %.3f cm" % xcuts[ii])
                    latex2.DrawLatex(0.2, 0.84, "efficiency: %.1f%%" % eff)
                    printme.append('  {} {}'.format(xcuts[ii], eff))

                elif "zresidual" in hname:
                    line2.DrawLine(zcuts[ii], 0, zcuts[ii], ymax)
                    line2.DrawLine(-zcuts[ii], 0, -zcuts[ii], ymax)

                    eff = h1.Integral(h1.FindFixBin(-zcuts[ii]), h1.FindFixBin(zcuts[ii])) / h1.Integral(0,999999) * 100.
                    latex2.DrawLatex(0.2, 0.88, "cut:  #Delta < %.3f cm" % zcuts[ii])
                    latex2.DrawLatex(0.2, 0.84, "efficiency: %.1f%%" % eff)
                    printme.append('  {} {}'.format(zcuts[ii], eff))

                CMS_label()
                save(imgdir, "multiplicity_"+h0.GetName())

        print '\n'.join(printme)
        donotdelete = []
        return donotdelete


    histos = bookLayer()
    projectLayer(tree, tree2, histos, nentries=nentries)
    d = drawLayer(histos, imgdir)


if sections["multiplicity2"]:

    in_quantiles = array('d', [0.0015, 0.025, 0.16, 0.5, 0.84, 0.975, 0.9985])
    quantiles = array('d', [0., 0., 0., 0., 0., 0., 0.])

    phimaxs = [0.006, 0.008, 0.010, 0.014, 0.018, 0.024]

    def bookLayer():
        histos = {}
        for i in xrange(5,11):
            layer = "b%i" % i
            ii = i-5

            h = TH1F("phiresidual_%s" % layer, "; stub #phi_{%s} - ideal #phi_{%s}; Entries" % (layer, layer), 240, -phimaxs[ii], phimaxs[ii])
            histos[h.GetName()] = h
        return histos

    def projectLayer(tree, histos, pt, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"    , 1)
        tree.SetBranchStatus("genParts_eta"   , 1)
        tree.SetBranchStatus("genParts_phi"   , 1)
        tree.SetBranchStatus("genParts_vz"    , 1)
        tree.SetBranchStatus("genParts_charge", 1)
        tree.SetBranchStatus("TTStubs_r"      , 1)
        tree.SetBranchStatus("TTStubs_phi"    , 1)
        tree.SetBranchStatus("TTStubs_z"      , 1)
        tree.SetBranchStatus("TTStubs_modId"  , 1)

        for i_ievt, ievt in enumerate(tree):
            simPt, simEta, simPhi, simVZ, simCharge = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0], ievt.genParts_vz[0], ievt.genParts_charge[0]
            if simPt < float(pt):  continue

            # Exactly 6 stubs
            if len(ievt.TTStubs_r) == 6:
                for r, phi, z, moduleId in izip(ievt.TTStubs_r, ievt.TTStubs_phi, ievt.TTStubs_z, ievt.TTStubs_modId):
                    lay = decodeLayer(moduleId)
                    layer = "b%i" % lay
                    if lay > 10:  print "ERROR! lay:", lay
                    ilay = lay - 5

                    deltaPhi = - 0.3*3.8*r*1e-2 * (simCharge/simPt) / 2.0
                    idealPhi = simPhi + deltaPhi
                    idealZ = r*sinh(simEta)  # cot(theta) = sinh(eta) = pz/pt

                    histos["phiresidual_%s" % layer].Fill(phi - idealPhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawLayer(histos, pt, imgdir):
        printme = []
        picks = [0.02, 0.25, 0.25]

        for i in xrange(5,11):
            layer = "b%i" % i
            ii = i-5
            r = b_rcoord_cmssw[ii]

            # residuals
            for hname in ["phiresidual_%s"]:
                h1 = histos[hname % layer]
                h1.SetLineWidth(2); h1.SetLineColor(col); h1.SetFillColor(fcol)
                ymax = h1.GetMaximum()
                h1.SetMinimum(0); h1.SetMaximum(ymax*1.4)
                h1.SetStats(0); h1.Draw("hist")

                moveLegend(0.56,0.84,0.92,0.94); legend.Clear()
                legend.AddEntry(h1, "#splitline{#mu=%.3e}{#sigma=%.3e}" % (h1.GetMean(),h1.GetRMS()), "f")
                legend.Draw()

                h1.GetQuantiles(len(quantiles), quantiles, in_quantiles)
                deltaPhi = - 0.3*3.8*r*1e-2 * (-1.0/pt) / 2.0
                ms0 = 0.0136/pt*sqrt(0.2)  # 13.6 MeV / p * sqrt(x/X_0)
                ms1 = 0.0136/pt*sqrt(0.2/6) * ii
                printme.append('{} {} {} {} {} {}'.format(h1.GetName(), h1.GetMean()+2.0*h1.GetRMS(), quantiles[5], deltaPhi, ms0, ms1))

                cut = deltaPhi * picks[0] + ms0 * picks[1] + ms1 * picks[2]
                line2.DrawLine(cut, 0, cut, ymax)
                line2.DrawLine(-cut, 0, -cut, ymax)

                eff = h1.Integral(h1.FindFixBin(-cut), h1.FindFixBin(cut)) / h1.Integral(0,999999) * 100.
                latex2.DrawLatex(0.2, 0.88, "cut:  #Delta < %.5f" % cut)
                latex2.DrawLatex(0.2, 0.84, "efficiency: %.1f%%" % eff)
                printme.append('  {} {}'.format(cut, eff))

                CMS_label()
                save(imgdir, "multiplicity2_"+h1.GetName())

        print '\n'.join(printme)
        donotdelete = []
        return donotdelete

    pt=2
    histos = bookLayer()
    projectLayer(tree, histos, pt, nentries=nentries)
    d = drawLayer(histos, pt, imgdir)
