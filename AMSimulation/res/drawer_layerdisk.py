#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
from math import pi

# ______________________________________________________________________________
# Configurations

sections = {}
sections["layerdisk"            ] = False
sections["triggertower"         ] = True
sections["rate_by_module"       ] = False
sections["rate_by_layer"        ] = False
sections["rate_by_layer2"       ] = False
sections["efficiency_by_module" ] = False
sections["efficiency_by_layer"  ] = False
sections["efficiency_by_layer2" ] = False
sections["efficiency_by_track"  ] = False
sections["nstubs_by_track"      ] = False
sections["nsuperstrips_by_track"] = False
sections["nmodules_by_track"    ] = False
sections["nlayers_by_track"     ] = False
sections["fixme"                ] = False

drawerInit = DrawerInit()
gStyle.SetNumberContours(100)
gStyle.SetPalette(55)  # rainbow color map

# Muon gun
#infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/SingleMuMinus_20140714/ntuple_1_1_1TX.root"]
#infiles = ["/uscms_data/d2/jiafu/L1TrackTrigger/CRAB_amsim/ntuple_1_1_1TX_redux.root"]
infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1/SingleMuMinus_20140821/SingleMu_E2023TTI_ntuple_1_1_oGy.root"]

col  = TColor.GetColor("#1f78b4")  # Paired
fcol = TColor.GetColor("#a6cee3")
imgdir = "figures_layerdisk/"


# Number of events
nentries = 1000000
#nentries = 50000

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
eb = EtaBinning("#eta", 60, -3, 3)
pb = EtaBinning("#phi", 64, -3.2, 3.2)


# ______________________________________________________________________________
# Geometry

if sections["layerdisk"]:

    if False:
        p0 = struct("layerdisk_nconnections", "dummy", "dummy",
            "# connections", 6, 0, 6, kBlack, blkrgb[2])
        histos = book([p0])

        connectmap = json.load(open("../data/module_connections.json"), object_pairs_hook=convert_key_to_int)
        for k, v in connectmap.iteritems():
            n = 0
            if k in ttrmap:
                assert(len(v) == len(ttrmap[k]))
                n = len(v)
            histos[0].h.Fill(n)
        draw(histos, ytitle="# modules")
        histos[0].h.SetMarkerSize(2)
        histos[0].h.Draw("same hist text0")
        save(imgdir, p0.name)

    if False:
        params = [
            struct("layerdisk_nconnections0_eta", "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, kBlack, blkrgb[0]),
            struct("layerdisk_nconnections1_eta", "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, kBlack, blkrgb[0]),
            struct("layerdisk_nconnections2_eta", "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, kBlack, blkrgb[0]),
            struct("layerdisk_nconnections3_eta", "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, kBlack, blkrgb[0]),
            struct("layerdisk_nconnections4_eta", "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, kBlack, blkrgb[0]),
            struct("layerdisk_nconnectionsA_eta", "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, kBlack, blkrgb[0]),
        ]
        for p0 in params:
            p0.fillstyle = 3004
        histos = book(params)

        coordmap = json.load(open("../data/module_coordinates.json"), object_pairs_hook=convert_key_to_int)
        for k, v in coordmap.iteritems():
            eta = v[2]

            n = 0
            if k in ttrmap:
                n = len(ttrmap[k])
            histos[n].h.Fill(eta)
            histos[len(params)-1].h.Fill(eta)

        for i in xrange(len(params)):
            if i < len(params)-1:
                ytitle = "# modules_{# connections=%i}" % i
            else:
                ytitle = "# modules"
            draw(histos[i:i+1], ytitle=ytitle)
            save(imgdir, params[i].name)

    if False:
        # By layers
        layers = []
        for i in xrange(5,11):
            layers.append(("b%i" % i, "Barrel %i" % (i-4)))
        for i in xrange(11,16):
            layers.append(("e%i" % i, "Endcap +%i" % (i-10)))
        for i in xrange(18,23):
            layers.append(("e%i" % i, "Endcap -%i" % (i-17)))

        coordmap = json.load(open("../data/module_coordinates.json"), object_pairs_hook=convert_key_to_int)
        coordmap_eta = {}
        coordmap_phi = {}
        for k, v in coordmap.iteritems():
            lay = decodeLayer(k)
            eta = v[2]
            phi = v[3]
            coordmap_eta.setdefault(lay, []).append(eta)
            coordmap_phi.setdefault(lay, []).append(phi)

        for l, ll in layers:
            p0 = struct2D("layerdisk_scatter_%s" % l, "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx * 8, eb.xlow, eb.xup, "module "+pb.xtitle, pb.nbinsx * 8, pb.xlow, pb.xup, kBlack, blkrgb[0])
            histos = book2D([p0])

            lay = int(l[1:])
            h = histos[0].h
            minEta, maxEta = 999999., -999999.
            minPhi, maxPhi = 999999., -999999.
            for eta, phi in izip(coordmap_eta[lay], coordmap_phi[lay]):
                h.Fill(eta, phi)

                if minEta > eta: minEta = eta
                if maxEta < eta: maxEta = eta
                if minPhi > phi: minPhi = phi
                if maxPhi < phi: maxPhi = phi

            h.SetStats(0)
            h.SetMarkerStyle(24); h.SetMarkerSize(0.5); h.SetMarkerColor(blkrgb[1])
            h.Draw()

            gPad.SetRightMargin(0.10)
            gPad.Modified(); gPad.Update()
            latex.SetTextSize(0.06)
            latex.DrawLatex(0.62, 0.88, ll)
            latex.SetTextSize(0.036)
            latex.DrawLatex(0.62, 0.83, "#eta = [%.2f,%.2f]" % (minEta, maxEta))
            latex.DrawLatex(0.62, 0.79, "#phi = [%.2f,%.2f]" % (minPhi, maxPhi))
            latex.DrawLatex(0.62, 0.75, "%i modules" % len(coordmap_eta[lay]))
            CMS_label()
            save(imgdir, p0.name)

    if False:
        # By towers
        towers = []
        for i in xrange(48):
            towers.append(("tt%i" % i, "Trigger tower %i" % i))

        coordmap = json.load(open("../data/module_coordinates.json"), object_pairs_hook=convert_key_to_int)
        coordmap_eta = {}
        coordmap_phi = {}
        for tt, tt_modules in ttmap.iteritems():
            for k in tt_modules:
                v = coordmap[k]
                eta = v[2]
                phi = v[3]
                coordmap_eta.setdefault(tt, []).append(eta)
                coordmap_phi.setdefault(tt, []).append(phi)

        for tt, ttt in towers:
            p0 = struct2D("layerdisk_scatter_%s" % tt, "dummy", "dummy",
                "module "+eb.xtitle, eb.nbinsx * 8, eb.xlow, eb.xup, "module "+pb.xtitle, pb.nbinsx * 8, pb.xlow, pb.xup, kBlack, blkrgb[0])
            histos = book2D([p0])

            tt = int(tt[2:])
            h = histos[0].h
            minEta, maxEta = 999999., -999999.
            minPhi, maxPhi = 999999., -999999.
            for eta, phi in izip(coordmap_eta[tt], coordmap_phi[tt]):
                h.Fill(eta, phi)

                if minEta > eta: minEta = eta
                if maxEta < eta: maxEta = eta
                if minPhi > phi: minPhi = phi
                if maxPhi < phi: maxPhi = phi

            h.SetStats(0)
            h.SetMarkerStyle(24); h.SetMarkerSize(0.5); h.SetMarkerColor(blkrgb[1])
            h.Draw()

            gPad.SetRightMargin(0.10)
            gPad.Modified(); gPad.Update()
            latex.SetTextSize(0.06)
            latex.DrawLatex(0.48, 0.88, ttt)
            latex.SetTextSize(0.036)
            latex.DrawLatex(0.48, 0.83, "#eta = [%.2f,%.2f]" % (minEta, maxEta))
            latex.DrawLatex(0.48, 0.79, "#phi = [%.2f,%.2f]" % (minPhi, maxPhi))
            latex.DrawLatex(0.48, 0.75, "%i modules" % len(coordmap_eta[tt]))
            CMS_label()
            save(imgdir, p0.name)

    if True:
        # By layers and towers
        p0 = struct2D("layerdisk_nsuperstrips_sssm", "dummy", "dummy",
            "Tower", 3, 0, 3, "Layer", 11+1, 0, 11+1, kBlack, blkrgb[0])
        p1 = struct2D("layerdisk_nsuperstrips_sslg", "dummy", "dummy",
            "Tower", 3, 0, 3, "Layer", 11+1, 0, 11+1, kBlack, blkrgb[0])
        p2 = struct2D("layerdisk_nsuperstrips_onlyphi_sssm", "dummy", "dummy",
            "Tower", 3, 0, 3, "Layer", 11+1, 0, 11+1, kBlack, blkrgb[0])
        p3 = struct2D("layerdisk_nsuperstrips_onlyphi_sslg", "dummy", "dummy",
            "Tower", 3, 0, 3, "Layer", 11+1, 0, 11+1, kBlack, blkrgb[0])
        histos = book2D([p0, p1, p2, p3])
        histos[0].caption = "ss32"
        histos[1].caption = "ss256"
        histos[2].caption = "ss32 (no z segmentation)"
        histos[3].caption = "ss256 (no z segmentation)"

        for tt, moduleIds in ttmap.iteritems():
            if tt not in [27, 35, 43]:
                continue

            moduleLadders = []
            for moduleId in moduleIds:
                s_tt = "tt" + str(tt)
                lay = decodeLayer(moduleId)
                if isBarrelModule(moduleId):
                    s_lay = "b" + str(lay)
                else:
                    s_lay = "e" + str(lay)
                if isPSModule(moduleId):
                    nsssms = (2 * 30)  # 30 = ceil(960./32)
                    nsslgs = (2 * 4)   #  4 = ceil(960./256)
                else:
                    nsssms = (2 * 32)  # 32 = ceil(1016./32)
                    nsslgs = (2 * 4)   #  4 = ceil(1016./256)

                lad = decodeLadder(moduleId)
                moduleLadder = lay*10000 + lad*100 + 0
                if isBarrelModule(moduleId):
                    if moduleLadder not in moduleLadders:
                        moduleLadders.append(moduleLadder)

                histos[0].h.Fill(s_tt, "all layers", nsssms)
                histos[1].h.Fill(s_tt, "all layers", nsslgs)
                histos[0].h.Fill(s_tt, s_lay, nsssms)
                histos[1].h.Fill(s_tt, s_lay, nsslgs)

            for moduleId in moduleLadders:
                s_tt = "tt" + str(tt)
                lay = decodeLayer(moduleId)
                if isBarrelModule(moduleId):
                    s_lay = "b" + str(lay)
                else:
                    s_lay = "e" + str(lay)
                nsssms = 32  # 32 = ceil(1016./32)
                nsslgs = 4   #  4 = ceil(1016./256)

                histos[2].h.Fill(s_tt, "all layers", nsssms)
                histos[3].h.Fill(s_tt, "all layers", nsslgs)
                histos[2].h.Fill(s_tt, s_lay, nsssms)
                histos[3].h.Fill(s_tt, s_lay, nsslgs)

        for histo in histos:
            h = histo.h
            #h.LabelsDeflate("X"); h.LabelsDeflate("Y"); h.LabelsOption("a")
            h.SetMarkerSize(1.5)
            h.Draw("TEXT")

            totalline = TLine(0, 1, 3, 1)
            totalline.SetLineStyle(7); totalline.SetLineColor(kGray)
            totalline.Draw()

            latex.SetTextSize(0.04)
            latex.DrawLatex(0.2, 0.88, "# of superstrips")
            latex.DrawLatex(0.2, 0.84, "using %s" % histo.caption)
            CMS_label()
            save(imgdir, histo.name)


if sections["triggertower"]:
    from math import cos, sin, tan, atan, exp
    gStyle.SetLabelSize(0.05, "Y")

    latex2 = TLatex()
    latex2.SetNDC(0)
    latex2.SetTextFont(42)
    latex2.SetTextSize(0.02)

    line2 = TLine()
    line2.SetLineColor(blkrgb[3])
    line2.SetLineStyle(1)
    line2.SetLineWidth(2)

    if True:
        for tt in [27]:
            nbins, maxR, maxZ = 500, 150., 300.
            p0 = struct2D("triggertower_scatter_PS_rhoz_tt%s" % tt, "dummy", "dummy",
                "module z [cm]", nbins, -maxZ, maxZ, "module r [cm]", nbins, 0, maxR, kBlack, blkrgb[0])
            p1 = struct2D("triggertower_scatter_PS_rhophi_tt%s" % tt, "dummy", "dummy",
                "module x [cm]", nbins, -maxR, maxR, "module y [cm]", nbins, -maxR, maxR, kBlack, blkrgb[0])
            p2 = struct2D("triggertower_scatter_2S_rhoz_tt%s" % tt, "dummy", "dummy",
                "module z [cm]", nbins, -maxZ, maxZ, "module r [cm]", nbins, 0, maxR, kBlack, blkrgb[0])
            p3 = struct2D("triggertower_scatter_2S_rhophi_tt%s" % tt, "dummy", "dummy",
                "module x [cm]", nbins, -maxR, maxR, "module y [cm]", nbins, -maxR, maxR, kBlack, blkrgb[0])
            histos = book2D([p0,p1,p2,p3])

            coordmap = json.load(open("../data/module_coordinates.json"), object_pairs_hook=convert_key_to_int)
            coordmap_x, coordmap_y, coordmap_z, coordmap_r, coordmap_eta, coordmap_phi = {}, {}, {}, {}, {}, {}
            for k in ttmap[tt]:
                lay = decodeLayer(k)
                v = coordmap[k]
                z, r, eta, phi = v[0:4]
                coordmap_x.setdefault(lay, []).append(r*cos(phi))
                coordmap_y.setdefault(lay, []).append(r*sin(phi))
                coordmap_z.setdefault(lay, []).append(z)
                coordmap_r.setdefault(lay, []).append(r)
                coordmap_eta.setdefault(lay, []).append(eta)
                coordmap_phi.setdefault(lay, []).append(phi)
                if isPSModule(k):
                    histos[0].h.Fill(z, r)
                    histos[1].h.Fill(r*cos(phi), r*sin(phi))
                else:
                    histos[2].h.Fill(z, r)
                    histos[3].h.Fill(r*cos(phi), r*sin(phi))

            # rho-z
            h = histos[0].h
            h.SetMarkerStyle(20); h.SetMarkerSize(0.6); h.SetMarkerColor(blkrgb[1]); h.SetLineColor(kWhite)
            h.SetStats(0); h.Draw()
            h.GetXaxis().SetRangeUser(min(coordmap_z[10])-20, max(coordmap_z[10])+60)
            h = histos[2].h
            h.SetMarkerStyle(20); h.SetMarkerSize(0.6); h.SetMarkerColor(blkrgb[2]); h.SetLineColor(kWhite)

            for eta in [x/10. for x in xrange(-2,11)]:
                theta = 2 * atan(exp(-eta))
                z = maxR / tan(theta)
                line.DrawLine(0, 0, z, maxR)
                z = (maxR-10) / tan(theta)
                latex2.DrawLatex(z-5, maxR-10, "#eta=%.1f" % eta)

            for eta in [0., 2.2/3]:
                theta = 2 * atan(exp(-eta))
                z = maxR / tan(theta)
                line2.DrawLine(0, 0, z, maxR)
                z = (maxR-20) / tan(theta)
                latex2.DrawLatex(z-5, maxR-20, "#eta=%.1f" % eta)

            histos[0].h.Draw("same")
            histos[2].h.Draw("same")

            moveLegend(0.66,0.16,0.94,0.42); legend.Clear()
            for i in xrange(10,4,-1):
                legend.AddEntry(h, "b%i #eta=[%.1f,%.1f]" % (i, min(coordmap_eta[i])-0.04, max(coordmap_eta[i])+0.05), "l")
            legend.Draw()
            CMS_label()
            save(imgdir, p0.name)

            # rho-phi
            h = histos[1].h
            h.SetMarkerStyle(20); h.SetMarkerSize(0.65); h.SetMarkerColor(blkrgb[1]); h.SetLineColor(kWhite)
            h.SetStats(0); h.Draw()
            minX = min(coordmap_x[5])-40
            maxX = minX+maxR
            h.GetXaxis().SetRangeUser(minX, maxX); h.GetYaxis().SetRangeUser(0, maxR); h.Draw()
            h = histos[3].h
            h.SetMarkerStyle(20); h.SetMarkerSize(0.6); h.SetMarkerColor(blkrgb[2]); h.SetLineColor(kWhite)

            for phi in [x/10. for x in xrange(6,19)]:
                if phi < 1.0:
                    line.DrawLine(0, 0, maxX, maxX * tan(phi))
                    latex2.DrawLatex(maxX-10, maxX * tan(phi)-5, "#phi=%.1f" % phi)
                elif phi < pi/2:
                    line.DrawLine(0, 0, maxR/tan(phi), maxR)
                    latex2.DrawLatex(maxR/tan(phi)-2, maxR-10, "#phi=%.1f" % phi)
                else:
                    line.DrawLine(0, 0, maxR/tan(phi), maxR)
                    latex2.DrawLatex(maxR/tan(phi)-2, maxR-10, "#phi=%.1f" % phi)

            for phi in [pi/4, pi/2]:
                if phi < 1.0:
                    line2.DrawLine(0, 0, maxX, maxX * tan(phi))
                    latex2.DrawLatex(maxX-20, maxX * tan(phi)-12, "#phi=#pi/%i" % int(pi/phi))
                elif phi < pi/2:
                    line2.DrawLine(0, 0, maxR/tan(phi), maxR)
                    latex2.DrawLatex(maxR/tan(phi)-5, maxR-20, "#phi=#pi/%i" % int(pi/phi))
                else:
                    line2.DrawLine(0, 0, maxR/tan(phi), maxR)
                    latex2.DrawLatex(maxR/tan(phi)-5, maxR-20, "#phi=#pi/%i" % int(pi/phi))

            histos[1].h.Draw("same")
            histos[3].h.Draw("same")

            moveLegend(0.66,0.16,0.94,0.42); legend.Clear()
            for i in xrange(10,4,-1):
                legend.AddEntry(h, "b%i #phi=[%.1f,%.1f]" % (i, min(coordmap_phi[i])-0.04, max(coordmap_phi[i])+0.05), "l")
            legend.Draw()
            CMS_label()
            save(imgdir, p1.name)


# ______________________________________________________________________________
# Rates

if sections["rate_by_module"]:

    def bookModule():
        histos = {}
        for i in xrange(5,11):  # barrel
            histos["b%i" % i] = TH2F("b%i" % i, "; Barrel module index; Barrel ladder index", b_nmods[i-5], 0, b_nmods[i-5], b_nlads[i-5], 0, b_nlads[i-5])

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            histos["e%i" % i] = TH2F("e%i" % i, "; r: Endcap ring index+%i; #theta: Endcap module index" % e_offset, 2*e_nrings[0], -e_nrings[0], e_nrings[0], 2*e_nrings[0], -e_nrings[0], e_nrings[0])  # a square frame

            for j in xrange(0,15):  # endcap rings
                histos["e%i_r%i" % (i,j)] = TH2F("e%i_r%i" % (i,j), "", e_nmods[j], 0, 2*pi, e_nrings[0], 0, e_nrings[0])

        histos["tt"] = TH2F("tt", "; Trigger tower eta index; Trigger tower phi index", tt_netas, 1, tt_netas+1, tt_nphis, 1, tt_nphis+1)
        return histos

    def projectModule(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        def fill(moduleId):
            lay = decodeLayer(moduleId)
            lad = decodeLadder(moduleId)
            mod = decodeModule(moduleId)

            if lay < 11:  # barrel
                histos["b%i" % (lay)].Fill(mod, lad, 1)

            else:         # endcap
                rad = 2*pi * (mod+0.5) / e_nmods[lad]
                histos["e%i_r%i" % (lay,lad)].Fill(rad, lad + e_offset, 1)

            if moduleId in ttrmap:
                for tt in ttrmap[moduleId]:
                    tteta = (tt / tt_nphis) + 1
                    ttphi = (tt % tt_nphis) + 1
                    histos["tt"].Fill(tteta, ttphi, 1)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId):
                if trkId == 1:
                    fill(moduleId)

        tree.SetBranchStatus("*", 1)
        return

    def drawModule(histos, imgdir):
        oldTextSize = latex.GetTextSize()
        latex.SetTextSize(0.06)

        # Get zmax
        zmax = 0
        for k, v in histos.iteritems():
            if k.startswith("b") or k.startswith("e"):
                zmax = max(zmax, v.GetMaximum())
        for k, v in histos.iteritems():
            if k.startswith("b") or k.startswith("e"):
                v.SetMinimum(0); v.SetMaximum(zmax)

        # Style fix
        for k, v in histos.iteritems():
            v.SetLabelSize(0.05, "XY")
            v.SetNdivisions(510, "XY")

        # Draw barrel
        for i in xrange(5,11):
            hname = "b%i" % i
            draw2D([histos[hname]], stats=False)
            latex.DrawLatex(0.62, 0.88, "Barrel %i" % (i-4))
            save(imgdir, "rate_by_module_%s" % hname)

        # Draw endcap
        for i in xrange(11,23):
            if i == 16 or i == 17:  continue
            hname = "e%i" % i
            draw2D([histos[hname]], stats=False, palette=False)

            for j in xrange(0,15):
                hname1 = "e%i_r%i" % (i,j)
                if j == 0:
                    histos[hname1].Draw("same pol colz")
                    gPad.Modified(); gPad.Update()
                    movePalette2D(histos[hname1])
                    gPad.Modified(); gPad.Update()
                else:
                    histos[hname1].Draw("same pol col")
            if i < 18:
                latex.DrawLatex(0.62, 0.88, "Endcap +%i" % (i-10))
            else:
                latex.DrawLatex(0.62, 0.88, "Endcap -%i" % (i-17))
            save(imgdir, "rate_by_module_%s" % hname)

        # Draw trigger tower
        hname = "tt"
        histos[hname].SetMinimum(0)
        draw2D([histos[hname]], stats=False)
        save(imgdir, "rate_by_module_%s" % hname)

        latex.SetTextSize(oldTextSize)
        donotdelete = []
        return donotdelete

    histos = bookModule()
    projectModule(tree, histos, nentries=nentries)
    d = drawModule(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["rate_by_layer"]:
    """rate vs eta"""

    def bookLayer():
        histos = {}
        for i in xrange(5,11):  # barrel
            histos["b%i" % i] = TH1F("b%i" % i, "; %s" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            histos["e%i" % i] = TH1F("e%i" % i, "; %s" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)

        for i in xrange(48):
            histos["tt%i" % i] = TH1F("tt%i" % i, "; %s" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
        return histos

    def projectLayer(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        def fill(moduleId, eta):
            lay = decodeLayer(moduleId)
            if lay < 11:  # barrel
                histos["b%i" % (lay)].Fill(eta)
            else:         # endcap
                histos["e%i" % (lay)].Fill(eta)

            if moduleId in ttrmap:
                for tt in ttrmap[moduleId]:
                    histos["tt%i" % (tt)].Fill(eta)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId):
                if trkId == 1:
                    fill(moduleId, theEta)

        tree.SetBranchStatus("*", 1)
        return

    def drawLayer(histos, imgdir):
        # Get ymax
        ymax = 0
        for k, v in histos.iteritems():
            ymax = max(ymax, v.GetMaximum())

        # Style fix
        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)

        # Draw barrel
        i = 5
        hname = "b%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
        h.Draw("hist")
        for i in xrange(5,11):
            hname1 = "b%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-5])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.70,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(5,11):
            leg1.AddEntry(histos["b%i" % i] , "Barrel layer %i" % (i-4), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "rate_by_layer_%s" % hname)

        # Draw endcap +
        i = 11
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
        h.Draw("hist")
        for i in xrange(11,16):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-11])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(11,16):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk +%i" % (i-10), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "rate_by_layer_%s" % hname)

        # Draw endcap -
        i = 18
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
        h.Draw("hist")
        for i in xrange(18,23):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-18])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(18,23):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk -%i" % (i-17), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "rate_by_layer_%s" % hname)

        # Make TLine
        boundaries = [-2.5, -1.3, -1.6, -0.5, -0.8, 0.2, -0.2, 0.8, 0.5, 1.6, 1.3, 2.5]
        tlines = [TLine(b, 0, b, ymax) for b in boundaries]
        for l in tlines:
            l.SetLineStyle(7); l.SetLineColor(kGray)

        # Draw trigger tower
        for iphi in xrange(8):
            i = 0
            hname = "tt%i" % (i*8 + iphi)
            h = histos[hname]
            h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
            h.Draw("hist")
            for l in tlines:
                l.Draw()
            for i in xrange(6):
                hname1 = "tt%i" % (i*8 + iphi)
                h = histos[hname1]
                h.SetLineColor(paletteSet1[i])
                h.Draw("same hist")

            leg1 = TLegend(0.56,0.70,0.94,0.94)
            leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
            for i in xrange(6):
                leg1.AddEntry(histos["tt%i" % (i*8 + iphi)] , "Trigger tower %i" % (i*8 + iphi), "l")
            leg1.Draw()
            CMS_label()
            save(imgdir, "rate_by_layer_%s" % hname)

        donotdelete = [tlines]
        return donotdelete

    histos = bookLayer()
    projectLayer(tree, histos, nentries=nentries)
    d = drawLayer(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["rate_by_layer2"]:
    """rate vs phi"""

    def bookLayer():
        histos = {}
        for i in xrange(5,11):  # barrel
            histos["b%i" % i] = TH1F("b%i" % i, "; %s" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            histos["e%i" % i] = TH1F("e%i" % i, "; %s" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)

        for i in xrange(48):
            histos["tt%i" % i] = TH1F("tt%i" % i, "; %s" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)
        return histos

    def projectLayer(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        def fill(moduleId, phi):
            lay = decodeLayer(moduleId)
            if lay < 11:  # barrel
                histos["b%i" % (lay)].Fill(phi)
            else:         # endcap
                histos["e%i" % (lay)].Fill(phi)

            if moduleId in ttrmap:
                for tt in ttrmap[moduleId]:
                    histos["tt%i" % (tt)].Fill(phi)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId):
                if trkId == 1:
                    fill(moduleId, thePhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawLayer(histos, imgdir):
        # Get ymax
        ymax = 0
        for k, v in histos.iteritems():
            ymax = max(ymax, v.GetMaximum())

        # Style fix
        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)

        # Draw barrel
        i = 5
        hname = "b%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
        h.Draw("hist")
        for i in xrange(5,11):
            hname1 = "b%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-5])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.70,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(5,11):
            leg1.AddEntry(histos["b%i" % i] , "Barrel layer %i" % (i-4), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "rate_by_layer2_%s" % hname)

        # Draw endcap +
        i = 11
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
        h.Draw("hist")
        for i in xrange(11,16):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-11])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(11,16):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk +%i" % (i-10), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "rate_by_layer2_%s" % hname)

        # Draw endcap -
        i = 18
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
        h.Draw("hist")
        for i in xrange(18,23):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-18])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(18,23):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk -%i" % (i-17), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "rate_by_layer2_%s" % hname)

        # Make TLine
        boundaries = [0.6, 1.7, 1.4, 2.5, 2.2, -3.1, 3.0, -2.3, -2.6, -1.5, -1.8, -0.7, -1.0, 0.1, -0.2, 0.9]
        tlines = [TLine(b, 0, b, ymax) for b in boundaries]
        for l in tlines:
            l.SetLineStyle(7); l.SetLineColor(kGray)

        # Draw trigger tower
        for ieta in xrange(6):
            i = 0
            hname = "tt%i" % (ieta*8 + i)
            h = histos[hname]
            h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.4)
            h.Draw("hist")
            for l in tlines:
                l.Draw()
            for i in xrange(8):
                hname1 = "tt%i" % (ieta*8 + i)
                h = histos[hname1]
                h.SetLineColor(paletteSet1[i])
                h.Draw("same hist")

            leg1 = TLegend(0.56,0.70,0.94,0.94)
            leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
            for i in xrange(8):
                leg1.AddEntry(histos["tt%i" % (ieta*8 + i)] , "Trigger tower %i" % (ieta*8 + i), "l")
            leg1.Draw()
            CMS_label()
            save(imgdir, "rate_by_layer2_%s" % hname)

        donotdelete = [tlines]
        return donotdelete

    histos = bookLayer()
    projectLayer(tree, histos, nentries=nentries)
    d = drawLayer(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["efficiency_by_module"]:

    def bookModule():
        histos = {}
        for i in xrange(5,11):  # barrel
            histos["b%i_denom" % i] = TH2F("b%i_denom" % i, "; Barrel module index; Barrel ladder index", b_nmods[i-5], 0, b_nmods[i-5], b_nlads[i-5], 0, b_nlads[i-5])
            histos["b%i_num"   % i] = TH2F("b%i_num"   % i, "; Barrel module index; Barrel ladder index", b_nmods[i-5], 0, b_nmods[i-5], b_nlads[i-5], 0, b_nlads[i-5])

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            histos["e%i" % i] = TH2F("e%i" % i, "; r: Endcap ring index+%i; #theta: Endcap module index" % e_offset, 2*e_nrings[0], -e_nrings[0], e_nrings[0], 2*e_nrings[0], -e_nrings[0], e_nrings[0])  # a square frame

            for j in xrange(0,15):  # endcap rings
                histos["e%i_r%i_denom" % (i,j)] = TH2F("e%i_r%i_denom" % (i,j), "", e_nmods[j], 0, 2*pi, e_nrings[0], 0, e_nrings[0])
                histos["e%i_r%i_num"   % (i,j)] = TH2F("e%i_r%i_num"   % (i,j), "", e_nmods[j], 0, 2*pi, e_nrings[0], 0, e_nrings[0])
        return histos

    def projectModule(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("TTStubs_hitChans"      , 1)
        tree.SetBranchStatus("simPixelDigis_trkId"   , 1)
        tree.SetBranchStatus("simPixelDigis_modId"   , 1)
        tree.SetBranchStatus("simPixelDigis_chan"    , 1)

        def fill(results_by_moduleId):
            for k, v in results_by_moduleId.iteritems():
                moduleId = k
                lay = decodeLayer(moduleId)
                lad = decodeLadder(moduleId)
                mod = decodeModule(moduleId)

                if lay < 11:  # barrel
                    histos["b%i_denom" % (lay)].Fill(mod, lad, 1)
                    if v:
                        histos["b%i_num" % (lay)].Fill(mod, lad, 1)
                else:         # endcap
                    rad = 2*pi * (mod+0.5) / e_nmods[lad]
                    histos["e%i_r%i_denom" % (lay,lad)].Fill(rad, lad + e_offset, 1)
                    if v:
                        histos["e%i_r%i_num" % (lay,lad)].Fill(rad, lad + e_offset, 1)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            lookup = {}
            results = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId, hitChans in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId, ievt.TTStubs_hitChans):
                if trkId == 1:
                    for chan in hitChans:
                        key = (moduleId, chan)
                        lookup[key] = True

            # Loop over digis
            for trkId, moduleId, chan in izip(ievt.simPixelDigis_trkId, ievt.simPixelDigis_modId, ievt.simPixelDigis_chan):
                if trkId == 1:
                    key = (moduleId, chan)
                    results[key] = (key in lookup)

            # Found?
            results_by_moduleId = {}
            for k, v in results.iteritems():
                moduleId = k[0]
                if moduleId not in results_by_moduleId:
                    results_by_moduleId[moduleId] = False  # init
                if v:
                    results_by_moduleId[moduleId] = True

            fill(results_by_moduleId)

        tree.SetBranchStatus("*", 1)
        return

    def drawModule(histos, imgdir):
        oldTextSize = latex.GetTextSize()
        latex.SetTextSize(0.06)

        # Style fix
        for k, v in histos.iteritems():
            v.SetLabelSize(0.05, "XY")
            v.SetNdivisions(510, "XY")

        # Make ratio plots
        zmax = 1
        for i in xrange(5,11):
            h = histos["b%i_num" % i].Clone("b%i" % i)
            h.Divide(histos["b%i_num" % i], histos["b%i_denom" % i], 1, 1, "b")
            h.SetMinimum(0); h.SetMaximum(zmax)
            histos[h.GetName()] = h

        for i in xrange(11,23):
            if i == 16 or i == 17:  continue

            for j in xrange(0,15):
                h = histos["e%i_r%i_num" % (i,j)].Clone("e%i_r%i" % (i,j))
                h.Divide(histos["e%i_r%i_num" % (i,j)], histos["e%i_r%i_denom" % (i,j)], 1, 1, "b")
                h.SetMinimum(0); h.SetMaximum(zmax)
                histos[h.GetName()] = h

        # Debug
        #for i in xrange(5,11):
        #    histos["b%i" % i].Print("all")

        # Draw barrel
        for i in xrange(5,11):
            hname = "b%i" % i
            draw2D([histos[hname]], stats=False)
            latex.DrawLatex(0.68, 0.88, "Barrel %i" % (i-4))
            save(imgdir, "efficiency_by_module_%s" % hname)

        # Draw endcap
        for i in xrange(11,23):
            if i == 16 or i == 17:  continue
            hname = "e%i" % i
            draw2D([histos[hname]], stats=False, palette=False)

            for j in xrange(0,15):
                hname1 = "e%i_r%i" % (i,j)
                if j == 0:
                    histos[hname1].Draw("same pol colz")
                    gPad.Modified(); gPad.Update()
                    movePalette2D(histos[hname1])
                    gPad.Modified(); gPad.Update()
                else:
                    histos[hname1].Draw("same pol col")
            if i < 18:
                latex.DrawLatex(0.62, 0.88, "Endcap +%i" % (i-10))
            else:
                latex.DrawLatex(0.62, 0.88, "Endcap -%i" % (i-17))
            save(imgdir, "efficiency_by_module_%s" % hname)

        latex.SetTextSize(oldTextSize)
        donotdelete = []
        return donotdelete

    histos = bookModule()
    projectModule(tree, histos, nentries=nentries)
    d = drawModule(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["efficiency_by_layer"]:
    """efficiency vs eta"""

    def bookLayer():
        histos = {}
        for i in xrange(5,11):  # barrel
            histos["b%i_denom" % i] = TH1F("b%i_denom" % i, "; %s; Layer efficiency" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["b%i_num"   % i] = TH1F("b%i_num"   % i, "; %s; Layer efficiency" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            histos["e%i_denom" % i] = TH1F("e%i_denom" % i, "; %s; Layer efficiency" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["e%i_num"   % i] = TH1F("e%i_num"   % i, "; %s; Layer efficiency" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
        return histos

    def projectLayer(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("TTStubs_hitChans"      , 1)
        tree.SetBranchStatus("simPixelDigis_trkId"   , 1)
        tree.SetBranchStatus("simPixelDigis_modId"   , 1)
        tree.SetBranchStatus("simPixelDigis_chan"    , 1)

        def fill(results_by_layer, eta):
            for k, v in results_by_layer.iteritems():
                lay = k

                if lay < 11:  # barrel
                    histos["b%i_denom" % (lay)].Fill(eta)
                    if v:
                        histos["b%i_num" % (lay)].Fill(eta)
                else:         # endcap
                    histos["e%i_denom" % (lay)].Fill(eta)
                    if v:
                        histos["e%i_num" % (lay)].Fill(eta)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            lookup = {}
            results = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId, hitChans in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId, ievt.TTStubs_hitChans):
                if trkId == 1:
                    for chan in hitChans:
                        key = (moduleId, chan)
                        lookup[key] = True

            # Loop over digis
            for trkId, moduleId, chan in izip(ievt.simPixelDigis_trkId, ievt.simPixelDigis_modId, ievt.simPixelDigis_chan):
                if trkId == 1:
                    key = (moduleId, chan)
                    results[key] = (key in lookup)

            results_by_layer = {}
            for k, v in results.iteritems():
                lay = decodeLayer(k[0])
                if lay not in results_by_layer:
                    results_by_layer[lay] = False  # init
                if v:
                    results_by_layer[lay] = True

            fill(results_by_layer, theEta)

        tree.SetBranchStatus("*", 1)
        return

    def drawLayer(histos, imgdir):
        # Style fix
        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)

        # Make TLine
        ymax = 1.
        oneline = TLine(eb.xlow, ymax, eb.xup, ymax)
        oneline.SetLineStyle(7); oneline.SetLineColor(kGray)

        # Make ratio plots
        for i in xrange(5,11):
            h = histos["b%i_num" % i].Clone("b%i" % i)
            h.Divide(histos["b%i_num" % i], histos["b%i_denom" % i], 1, 1, "b")
            histos[h.GetName()] = h

        for i in xrange(11,23):
            if i == 16 or i == 17:  continue
            h = histos["e%i_num" % i].Clone("e%i" % i)
            h.Divide(histos["e%i_num" % i], histos["e%i_denom" % i], 1, 1, "b")
            histos[h.GetName()] = h

        # Debug
        #for i in xrange(5,11):
        #    histos["b%i" % i].Print("all")

        # Draw barrel
        i = 5
        hname = "b%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
        h.Draw("hist")
        oneline.Draw()
        for i in xrange(5,11):
            hname1 = "b%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-5])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.70,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(5,11):
            leg1.AddEntry(histos["b%i" % i] , "Barrel layer %i" % (i-4), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "efficiency_by_layer_%s" % hname)

        # Draw endcap +
        i = 11
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
        h.Draw("hist")
        oneline.Draw()
        for i in xrange(11,16):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-11])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(11,16):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk +%i" % (i-10), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "efficiency_by_layer_%s" % hname)

        # Draw endcap -
        i = 18
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
        h.Draw("hist")
        oneline.Draw()
        for i in xrange(18,23):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-18])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(18,23):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk -%i" % (i-17), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "efficiency_by_layer_%s" % hname)

        donotdelete = []
        return donotdelete

    histos = bookLayer()
    projectLayer(tree, histos, nentries=nentries)
    d = drawLayer(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["efficiency_by_layer2"]:
    """efficiency vs phi"""

    def bookLayer():
        histos = {}
        for i in xrange(5,11):  # barrel
            histos["b%i_denom" % i] = TH1F("b%i_denom" % i, "; %s; Layer efficiency" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)
            histos["b%i_num"   % i] = TH1F("b%i_num"   % i, "; %s; Layer efficiency" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            histos["e%i_denom" % i] = TH1F("e%i_denom" % i, "; %s; Layer efficiency" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)
            histos["e%i_num"   % i] = TH1F("e%i_num"   % i, "; %s; Layer efficiency" % pb.xtitle, pb.nbinsx, pb.xlow, pb.xup)
        return histos

    def projectLayer(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("TTStubs_hitChans"      , 1)
        tree.SetBranchStatus("simPixelDigis_trkId"   , 1)
        tree.SetBranchStatus("simPixelDigis_modId"   , 1)
        tree.SetBranchStatus("simPixelDigis_chan"    , 1)

        def fill(results_by_layer, phi):
            for k, v in results_by_layer.iteritems():
                lay = k

                if lay < 11:  # barrel
                    histos["b%i_denom" % (lay)].Fill(phi)
                    if v:
                        histos["b%i_num" % (lay)].Fill(phi)
                else:         # endcap
                    histos["e%i_denom" % (lay)].Fill(phi)
                    if v:
                        histos["e%i_num" % (lay)].Fill(phi)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            lookup = {}
            results = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId, hitChans in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId, ievt.TTStubs_hitChans):
                if trkId == 1:
                    for chan in hitChans:
                        key = (moduleId, chan)
                        lookup[key] = True

            # Loop over digis
            for trkId, moduleId, chan in izip(ievt.simPixelDigis_trkId, ievt.simPixelDigis_modId, ievt.simPixelDigis_chan):
                if trkId == 1:
                    key = (moduleId, chan)
                    results[key] = (key in lookup)

            results_by_layer = {}
            for k, v in results.iteritems():
                lay = decodeLayer(k[0])
                if lay not in results_by_layer:
                    results_by_layer[lay] = False  # init
                if v:
                    results_by_layer[lay] = True

            fill(results_by_layer, thePhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawLayer(histos, imgdir):
        # Style fix
        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)

        # Make TLine
        ymax = 1.
        oneline = TLine(eb.xlow, ymax, eb.xup, ymax)
        oneline.SetLineStyle(7); oneline.SetLineColor(kGray)

        # Make ratio plots
        for i in xrange(5,11):
            h = histos["b%i_num" % i].Clone("b%i" % i)
            h.Divide(histos["b%i_num" % i], histos["b%i_denom" % i], 1, 1, "b")
            histos[h.GetName()] = h

        for i in xrange(11,23):
            if i == 16 or i == 17:  continue
            h = histos["e%i_num" % i].Clone("e%i" % i)
            h.Divide(histos["e%i_num" % i], histos["e%i_denom" % i], 1, 1, "b")
            histos[h.GetName()] = h

        # Debug
        #for i in xrange(5,11):
        #    histos["b%i" % i].Print("all")

        # Draw barrel
        i = 5
        hname = "b%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
        h.Draw("hist")
        oneline.Draw()
        for i in xrange(5,11):
            hname1 = "b%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-5])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.70,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(5,11):
            leg1.AddEntry(histos["b%i" % i] , "Barrel layer %i" % (i-4), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "efficiency_by_layer2_%s" % hname)

        # Draw endcap +
        i = 11
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
        h.Draw("hist")
        oneline.Draw()
        for i in xrange(11,16):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-11])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(11,16):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk +%i" % (i-10), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "efficiency_by_layer2_%s" % hname)

        # Draw endcap -
        i = 18
        hname = "e%i" % i
        h = histos[hname]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
        h.Draw("hist")
        oneline.Draw()
        for i in xrange(18,23):
            hname1 = "e%i" % i
            h = histos[hname1]
            h.SetLineColor(paletteSet1[i-18])
            h.Draw("same hist")

        leg1 = TLegend(0.56,0.74,0.94,0.94)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        for i in xrange(18,23):
            leg1.AddEntry(histos["e%i" % i] , "Endcap disk -%i" % (i-17), "l")
        leg1.Draw()
        CMS_label()
        save(imgdir, "efficiency_by_layer2_%s" % hname)

        donotdelete = []
        return donotdelete

    histos = bookLayer()
    projectLayer(tree, histos, nentries=nentries)
    d = drawLayer(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["efficiency_by_track"]:

    def bookTrack():
        histos = {}
        for i in xrange(3):
            histos["l6_n%i_denom" % i] = TH1F("l6_n%i_denom" % i, "; %s; Efficiency of #geq6 layers" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["l6_n%i_num"   % i] = TH1F("l6_n%i_num"   % i, "; %s; Efficiency of #geq6 layers" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["l5_n%i_denom" % i] = TH1F("l5_n%i_denom" % i, "; %s; Efficiency of #geq5 layers" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["l5_n%i_num"   % i] = TH1F("l5_n%i_num"   % i, "; %s; Efficiency of #geq5 layers" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["l4_n%i_denom" % i] = TH1F("l4_n%i_denom" % i, "; %s; Efficiency of #geq4 layers" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
            histos["l4_n%i_num"   % i] = TH1F("l4_n%i_num"   % i, "; %s; Efficiency of #geq4 layers" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup)
        return histos

    def projectTrack(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("TTStubs_hitChans"      , 1)
        tree.SetBranchStatus("simPixelDigis_trkId"   , 1)
        tree.SetBranchStatus("simPixelDigis_modId"   , 1)
        tree.SetBranchStatus("simPixelDigis_chan"    , 1)

        def fill(results_by_mlayer, pt, eta):
            nlayers = len(results_by_mlayer)
            #assert(nlayers <= 6)

            eff = sum(results_by_mlayer.values())

            if pt >= 20:
                if nlayers >= 6:
                    histos["l6_n0_denom"].Fill(eta)
                    if eff >= 6:
                        histos["l6_n0_num"].Fill(eta)
                if nlayers >= 5:
                    histos["l5_n0_denom"].Fill(eta)
                    if eff >= 5:
                        histos["l5_n0_num"].Fill(eta)
                if nlayers >= 4:
                    histos["l4_n0_denom"].Fill(eta)
                    if eff >= 4:
                        histos["l4_n0_num"].Fill(eta)
            elif pt >= 5:
                if nlayers >= 6:
                    histos["l6_n1_denom"].Fill(eta)
                    if eff >= 6:
                        histos["l6_n1_num"].Fill(eta)
                if nlayers >= 5:
                    histos["l5_n1_denom"].Fill(eta)
                    if eff >= 5:
                        histos["l5_n1_num"].Fill(eta)
                if nlayers >= 4:
                    histos["l4_n1_denom"].Fill(eta)
                    if eff >= 4:
                        histos["l4_n1_num"].Fill(eta)
            elif pt >= 2:
                if nlayers >= 6:
                    histos["l6_n2_denom"].Fill(eta)
                    if eff >= 6:
                        histos["l6_n2_num"].Fill(eta)
                if nlayers >= 5:
                    histos["l5_n2_denom"].Fill(eta)
                    if eff >= 5:
                        histos["l5_n2_num"].Fill(eta)
                if nlayers >= 4:
                    histos["l4_n2_denom"].Fill(eta)
                    if eff >= 4:
                        histos["l4_n2_num"].Fill(eta)
            return

        # Merged layers
        layermap = {
            5:0, 6:1, 7:2, 8:3, 9:4, 10:5,
            11:5, 12:4, 13:3, 14:2, 15:1,
            18:5, 19:4, 20:3, 21:2, 22:1,
        }

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            lookup = {}
            results = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId, hitChans in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId, ievt.TTStubs_hitChans):
                if trkId == 1:
                    for chan in hitChans:
                        key = (moduleId, chan)
                        lookup[key] = True

            # Loop over digis
            for trkId, moduleId, chan in izip(ievt.simPixelDigis_trkId, ievt.simPixelDigis_modId, ievt.simPixelDigis_chan):
                if trkId == 1:
                    key = (moduleId, chan)
                    results[key] = (key in lookup)

            # Found?
            results_by_mlayer = {}
            for k, v in results.iteritems():
                lay = decodeLayer(k[0])
                mlay = layermap[lay]
                if mlay not in results_by_mlayer:
                    results_by_mlayer[mlay] = False  # init
                if v:
                    results_by_mlayer[mlay] = True

            fill(results_by_mlayer, thePt, theEta)

        tree.SetBranchStatus("*", 1)
        return

    def drawTrack(histos, imgdir):
        # Style fix
        for k, v in histos.iteritems():
            v.SetLineWidth(2); v.SetMarkerSize(0)

        # Make TLine
        ymax = 1.
        oneline = TLine(eb.xlow, ymax, eb.xup, ymax)
        oneline.SetLineStyle(7); oneline.SetLineColor(kGray)

        # Make TBox
        tboxes = [
            TBox(-0.90, 0, -1.20, ymax),
            TBox( 0.90, 0,  1.20, ymax),
            #TBox(-1.55, 0, -1.60, ymax),
            #TBox( 1.55, 0,  1.60, ymax),
        ]
        for box in tboxes:
            box.SetFillStyle(3004)
            box.SetFillColor(kGray)

        # Make ratio plots
        for lx in ["l6", "l5", "l4"]:
            for i in xrange(3):
                h = histos["%s_n%i_num" % (lx,i)].Clone("%s_n%i" % (lx,i))
                h.Divide(histos["%s_n%i_num" % (lx,i)], histos["%s_n%i_denom" % (lx,i)], 1, 1, "b")
                h.SetLineColor(blkrgb[i % 3])

                if i == 0:
                    h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.5)
                    h.Draw("hist")
                    oneline.Draw()
                    for box in tboxes:
                        box.Draw()
                h.Draw("same hist")
                histos[h.GetName()] = h

            leg1 = TLegend(0.56,0.14,0.94,0.34)
            leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
            leg1.AddEntry(histos["%s_n0" % lx], "20 #leq p_{T} < #infty  GeV", "l")
            leg1.AddEntry(histos["%s_n1" % lx], "  5 #leq p_{T} < 20 GeV"    , "l")
            leg1.AddEntry(histos["%s_n2" % lx], "  2 #leq p_{T} <   5 GeV"   , "l")
            leg1.Draw()
            CMS_label()
            save(imgdir, "efficiency_by_track_%s" % lx)

        donotdelete = [leg1, tboxes]
        return donotdelete

    histos = bookTrack()
    projectTrack(tree, histos, nentries=nentries)
    d = drawTrack(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["nstubs_by_track"]:
    """Count numbers of digis, of clusters, of stubs"""

    def bookTrack():
        histos = {}
        for i in xrange(3):
            histos["ndigis%i" % i] = TProfile("ndigis%i" % i, "; %s; <# digis>"    % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
            histos["ncluss%i" % i] = TProfile("ncluss%i" % i, "; %s; <# clusters>" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
            histos["nstubs%i" % i] = TProfile("nstubs%i" % i, "; %s; <# stubs>"    % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
        histos["ndigis_2D"] = TProfile2D("ndigis_2D", "; %s; %s; <# digis>"    % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        histos["ncluss_2D"] = TProfile2D("ncluss_2D", "; %s; %s; <# clusters>" % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        histos["nstubs_2D"] = TProfile2D("nstubs_2D", "; %s; %s; <# stubs>"    % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        return histos

    def projectTrack(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("simPixelDigis_trkId"   , 1)
        tree.SetBranchStatus("TTClusters_trkId"      , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)

        def fill(ndigis, ncluss, nstubs, pt, eta, phi):
            if pt >= 20:
                histos["ndigis0"].Fill(eta, ndigis)
                histos["ncluss0"].Fill(eta, ncluss)
                histos["nstubs0"].Fill(eta, nstubs)
            elif pt >= 5:
                histos["ndigis1"].Fill(eta, ndigis)
                histos["ncluss1"].Fill(eta, ncluss)
                histos["nstubs1"].Fill(eta, nstubs)
            elif pt >= 2:
                histos["ndigis2"].Fill(eta, ndigis)
                histos["ncluss2"].Fill(eta, ncluss)
                histos["nstubs2"].Fill(eta, nstubs)

            if pt >= 20:
                histos["ndigis_2D"].Fill(eta, phi, ndigis)
                histos["ncluss_2D"].Fill(eta, phi, ncluss)
                histos["nstubs_2D"].Fill(eta, phi, nstubs)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            ndigis, ncluss, nstubs = 0, 0, 0

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over digis:
            for trkId in ievt.simPixelDigis_trkId:
                if trkId == 1:
                    ndigis += 1

            # Loop over clusters:
            for trkId in ievt.TTClusters_trkId:
                if trkId == 1:
                    ncluss += 1

            # Loop over stubs
            for trkId in ievt.TTStubs_trkId:
                if trkId == 1:
                    nstubs += 1

            fill(ndigis, ncluss, nstubs, thePt, theEta, thePhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawTrack(histos, imgdir):
        oldTextSize = latex.GetTextSize()
        latex.SetTextSize(0.06)

        leg1 = TLegend(0.56,0.14,0.94,0.34)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)

        for hvar in ["ndigis", "ncluss", "nstubs"]:
            for i in xrange(3):
                hname = hvar + "%i" % i
                h = histos[hname]
                h.SetMarkerColor(blkrgb[i % 3]); h.SetLineColor(blkrgb[i % 3]); h.SetMarkerSize(1.3)

                if i == 0:
                    ymax = h.GetMaximum()
                    h.SetStats(0); h.SetMinimum(0); h.SetMaximum(round(ymax * 1.333))
                    h.Draw()
                h.Draw("same")

            leg1.Clear()
            leg1.AddEntry(histos[hvar+"0"], "20 #leq p_{T} < #infty  GeV", "p")
            leg1.AddEntry(histos[hvar+"1"], "  5 #leq p_{T} < 20 GeV"    , "p")
            leg1.AddEntry(histos[hvar+"2"], "  2 #leq p_{T} <   5 GeV"   , "p")
            leg1.Draw()
            CMS_label()
            save(imgdir, "nstubs_by_track_%s" % hvar)

        # TH2F
        for hvar in ["ndigis_2D", "ncluss_2D", "nstubs_2D"]:
            hname = hvar
            h = histos[hname]
            ymax = h.GetMaximum()
            h.SetStats(0); h.SetMinimum(0); h.SetMaximum(round(ymax * 1.333))
            draw2D([h], stats=False)
            latex.DrawLatex(0.58, 0.88, "p_{T} > 20 GeV")
            save(imgdir, "nstubs_by_track_%s" % hvar)

        latex.SetTextSize(oldTextSize)
        donotdelete = [leg1]
        return donotdelete

    histos = bookTrack()
    projectTrack(tree, histos, nentries=nentries)
    d = drawTrack(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["nsuperstrips_by_track"]:
    """Count numbers of superstrips"""

    def encode_sssm(moduleId, col, row):
        """Small superstrip size: ss32"""

        if isPSModule(moduleId):
            col >>= 4
        row >>= 5
        s = (moduleId << 11) | (col << 10) | row
        return s

    def encode_sslg(moduleId, col, row):
        """Large superstrip size: ss256"""

        if isPSModule(moduleId):
            col >>= 4
        row >>= 8
        s = (moduleId << 11) | (col << 10) | row
        return s

    def encode_chip(moduleId, col, row):
        """# chips for MPA & CBC = 2 x 8"""

        if isPSModule(moduleId):
            col >>= 4
            row /= 120  # 120 = 960/8
        else:
            row /= 127  # 127 = 1016/8
        s = (moduleId << 11) | (col << 10) | row
        return s

    def encode_conc(moduleId, col, row):
        """# concentrator (CIC) = 2"""

        if isPSModule(moduleId):
            col >>= 4
        row = 0
        #assert(col == 0 or col == 1)
        s = (moduleId << 11) | (col << 10) | row
        return s

    def bookTrack():
        histos = {}
        for i in xrange(3):
            histos["nsssms%i" % i] = TProfile("nsssms%i" % i, "; %s; <# superstrip-32's>"  % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
            histos["nsslgs%i" % i] = TProfile("nsslgs%i" % i, "; %s; <# superstrip-256's>" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
            histos["nchips%i" % i] = TProfile("nchips%i" % i, "; %s; <# read-out chips>"   % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
            histos["nconcs%i" % i] = TProfile("nconcs%i" % i, "; %s; <# concentrators>"    % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
        histos["nsssms_2D"] = TProfile2D("nsssms_2D", "; %s; %s; <# superstrip-32's>"  % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        histos["nsslgs_2D"] = TProfile2D("nsslgs_2D", "; %s; %s; <# superstrip-256's>" % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        histos["nchips_2D"] = TProfile2D("nchips_2D", "; %s; %s; <# chips>"            % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        histos["nconcs_2D"] = TProfile2D("nconcs_2D", "; %s; %s; <# concs>"            % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        return histos

    def projectTrack(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)
        tree.SetBranchStatus("TTStubs_coordx"        , 1)
        tree.SetBranchStatus("TTStubs_coordy"        , 1)

        def fill(nsssms, nsslgs, nchips, nconcs, pt, eta, phi):
            if pt >= 20:
                histos["nsssms0"].Fill(eta, nsssms)
                histos["nsslgs0"].Fill(eta, nsslgs)
                histos["nchips0"].Fill(eta, nchips)
                histos["nconcs0"].Fill(eta, nconcs)
            elif pt >= 5:
                histos["nsssms1"].Fill(eta, nsssms)
                histos["nsslgs1"].Fill(eta, nsslgs)
                histos["nchips1"].Fill(eta, nchips)
                histos["nconcs1"].Fill(eta, nconcs)
            elif pt >= 2:
                histos["nsssms2"].Fill(eta, nsssms)
                histos["nsslgs2"].Fill(eta, nsslgs)
                histos["nchips2"].Fill(eta, nchips)
                histos["nconcs2"].Fill(eta, nconcs)

            if pt >= 20:
                histos["nsssms_2D"].Fill(eta, phi, nsssms)
                histos["nsslgs_2D"].Fill(eta, phi, nsslgs)
                histos["nchips_2D"].Fill(eta, phi, nchips)
                histos["nconcs_2D"].Fill(eta, phi, nconcs)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            results_sssm = {}
            results_sslg = {}
            results_chip = {}
            results_conc = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId, coordx, coordy in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId, ievt.TTStubs_coordx, ievt.TTStubs_coordy):
                if trkId == 1:
                    col = halfStripRound(coordy)
                    row = halfStripRound(coordx)
                    col >>= 1
                    row >>= 1

                    sssm = encode_sssm(moduleId, col, row)
                    sslg = encode_sslg(moduleId, col, row)
                    chip = encode_chip(moduleId, col, row)
                    conc = encode_conc(moduleId, col, row)
                    results_sssm[sssm] = True
                    results_sslg[sslg] = True
                    results_chip[chip] = True
                    results_conc[conc] = True

            # Count
            nsssms = len(results_sssm)
            nsslgs = len(results_sslg)
            nchips = len(results_chip)
            nconcs = len(results_conc)

            fill(nsssms, nsslgs, nchips, nconcs, thePt, theEta, thePhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawTrack(histos, imgdir):
        oldTextSize = latex.GetTextSize()
        latex.SetTextSize(0.06)

        leg1 = TLegend(0.56,0.14,0.94,0.34)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)

        for hvar in ["nsssms", "nsslgs", "nchips", "nconcs"]:
            for i in xrange(3):
                hname = hvar + "%i" % i
                h = histos[hname]
                h.SetMarkerColor(blkrgb[i % 3]); h.SetLineColor(blkrgb[i % 3]); h.SetMarkerSize(1.3)

                if i == 0:
                    ymax = h.GetMaximum()
                    h.SetStats(0); h.SetMinimum(0); h.SetMaximum(round(ymax * 1.333))
                    h.Draw()
                h.Draw("same")

            leg1.Clear()
            leg1.AddEntry(histos[hvar+"0"], "20 #leq p_{T} < #infty  GeV", "p")
            leg1.AddEntry(histos[hvar+"1"], "  5 #leq p_{T} < 20 GeV"    , "p")
            leg1.AddEntry(histos[hvar+"2"], "  2 #leq p_{T} <   5 GeV"   , "p")
            leg1.Draw()
            CMS_label()
            save(imgdir, "nsuperstrips_by_track_%s" % hvar)

        # TH2F
        for hvar in ["nsssms_2D", "nsslgs_2D", "nchips_2D", "nconcs_2D"]:
            hname = hvar
            h = histos[hname]
            ymax = h.GetMaximum()
            h.SetStats(0); h.SetMinimum(0); h.SetMaximum(round(ymax * 1.333))
            draw2D([h], stats=False)
            latex.DrawLatex(0.58, 0.88, "p_{T} > 20 GeV")
            save(imgdir, "nsuperstrips_by_track_%s" % hvar)

        latex.SetTextSize(oldTextSize)
        donotdelete = [leg1]
        return donotdelete

    histos = bookTrack()
    projectTrack(tree, histos, nentries=nentries)
    d = drawTrack(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["nmodules_by_track"]:
    """Count multiple stubs in one module as one"""

    def bookTrack():
        histos = {}
        for i in xrange(3):
            histos["n%i" % i] = TProfile("n%i" % i, "; %s; <# modules>" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
        histos["n_2D"] = TProfile2D("n_2D", "; %s; %s; <# modules>" % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        return histos

    def projectTrack(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        def fill(nmodules, pt, eta, phi):
            if pt >= 20:
                histos["n0"].Fill(eta, nmodules)
            elif pt >= 5:
                histos["n1"].Fill(eta, nmodules)
            elif pt >= 2:
                histos["n2"].Fill(eta, nmodules)

            if pt >= 20:
                histos["n_2D"].Fill(eta, phi, nmodules)
            return

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            results = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId):
                if trkId == 1:
                    results[moduleId] = True

            # Count
            nmodules = len(results)

            fill(nmodules, thePt, theEta, thePhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawTrack(histos, imgdir):
        oldTextSize = latex.GetTextSize()
        latex.SetTextSize(0.06)

        for i in xrange(3):
            hname = "n%i" % i
            h = histos[hname]
            h.SetMarkerColor(blkrgb[i % 3]); h.SetLineColor(blkrgb[i % 3]); h.SetMarkerSize(1.3)

            if i == 0:
                ymax = 10
                h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.2)
                h.Draw()
            h.Draw("same")

        leg1 = TLegend(0.56,0.14,0.94,0.34)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        leg1.AddEntry(histos["n0"], "20 #leq p_{T} < #infty  GeV", "p")
        leg1.AddEntry(histos["n1"], "  5 #leq p_{T} < 20 GeV"    , "p")
        leg1.AddEntry(histos["n2"], "  2 #leq p_{T} <   5 GeV"   , "p")
        leg1.Draw()
        CMS_label()
        save(imgdir, "nmodules_by_track_1D")

        # TH2F
        hname = "n_2D"
        h = histos[hname]
        ymax = 10
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.2)
        draw2D([h], stats=False)
        latex.DrawLatex(0.58, 0.88, "p_{T} > 20 GeV")
        save(imgdir, "nmodules_by_track_2D")

        latex.SetTextSize(oldTextSize)
        donotdelete = [leg1]
        return donotdelete

    histos = bookTrack()
    projectTrack(tree, histos, nentries=nentries)
    d = drawTrack(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["nlayers_by_track"]:
    """Count multiple stubs in one layer as one"""

    def bookTrack():
        histos = {}
        for i in xrange(3):
            histos["n%i" % i] = TProfile("n%i" % i, "; %s; <# layers>" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
            histos["n%i_tt" % i] = TProfile("n%i_tt" % i, "; %s; <# layers>" % eb.xtitle, eb.nbinsx, eb.xlow, eb.xup, 0, 100000)
        histos["n_2D"] = TProfile2D("n_2D", "; %s; %s; <# layers>" % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        histos["n_tt_2D"] = TProfile2D("n_tt_2D", "; %s; %s; <# layers>" % (eb.xtitle, pb.xtitle), eb.nbinsx, eb.xlow, eb.xup, pb.nbinsx, pb.xlow, pb.xup, 0, 100000)
        return histos

    def projectTrack(tree, histos, nentries=1000000000, sim=True):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("genParts_pt"           , 1)
        tree.SetBranchStatus("genParts_eta"          , 1)
        tree.SetBranchStatus("genParts_phi"          , 1)
        tree.SetBranchStatus("TTStubs_trkId"         , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        def fill(nlayers, nlayers_tt, pt, eta, phi):
            if pt >= 20:
                histos["n0"].Fill(eta, nlayers)
                histos["n0_tt"].Fill(eta, nlayers_tt)
            elif pt >= 5:
                histos["n1"].Fill(eta, nlayers)
                histos["n1_tt"].Fill(eta, nlayers_tt)
            elif pt >= 2:
                histos["n2"].Fill(eta, nlayers)
                histos["n2_tt"].Fill(eta, nlayers_tt)

            if pt >= 20:
                histos["n_2D"].Fill(eta, phi, nlayers)
                histos["n_tt_2D"].Fill(eta, phi, nlayers_tt)
            return

        # Merged layers
        layermap = {
            5:0, 6:1, 7:2, 8:3, 9:4, 10:5,
            11:5, 12:4, 13:3, 14:2, 15:1,
            18:5, 19:4, 20:3, 21:2, 22:1,
        }

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            results = {}
            results_tt = {}

            # Check track pt, eta, phi
            thePt, theEta, thePhi = ievt.genParts_pt[0], ievt.genParts_eta[0], ievt.genParts_phi[0]
            if not (thePt >= 2 and abs(theEta) < 2.5):
                print "cut:", thePt, theEta, thePhi
                continue

            # Loop over stubs
            for trkId, moduleId in izip(ievt.TTStubs_trkId, ievt.TTStubs_modId):
                if trkId == 1:
                    lay = decodeLayer(moduleId)
                    mlay = layermap[lay]
                    results[mlay] = True

                    # For results_tt, append not-yet-found trigger towers,
                    # or put all the trigger towers,
                    # or put nothing
                    if moduleId in ttrmap:
                        if mlay in results_tt:
                            for tt in ttrmap[moduleId]:
                                if tt not in results_tt[mlay]:
                                    results_tt[mlay].append(tt)
                        else:
                            results_tt[mlay] = ttrmap[moduleId]
                    else:
                        results_tt[mlay] = []

            # Count
            nlayers = len(results)
            results_tt_aux = {}
            for k, v in results_tt.iteritems():
                for tt in v:
                    if tt in results_tt_aux:
                        results_tt_aux[tt] += 1
                    else:
                        results_tt_aux[tt] = 1
            nlayers_tt = max([0] + results_tt_aux.values())  # avoid empty sequence
            #assert(nlayers_tt <= nlayers)

            fill(nlayers, nlayers_tt, thePt, theEta, thePhi)

        tree.SetBranchStatus("*", 1)
        return

    def drawTrack(histos, imgdir):
        oldTextSize = latex.GetTextSize()
        latex.SetTextSize(0.04)

        for i in xrange(3):
            hname = "n%i" % i
            h = histos[hname]
            h.SetMarkerColor(blkrgb[i % 3]); h.SetLineColor(blkrgb[i % 3]); h.SetMarkerSize(1.3)

            if i == 0:
                ymax = 10
                h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.2)
                h.Draw()
            h.Draw("same")

        leg1 = TLegend(0.56,0.14,0.94,0.34)
        leg1.SetFillStyle(0); leg1.SetLineColor(0); leg1.SetShadowColor(0); leg1.SetBorderSize(0)
        leg1.AddEntry(histos["n0"], "20 #leq p_{T} < #infty  GeV", "p")
        leg1.AddEntry(histos["n1"], "  5 #leq p_{T} < 20 GeV"    , "p")
        leg1.AddEntry(histos["n2"], "  2 #leq p_{T} <   5 GeV"   , "p")
        leg1.Draw()
        CMS_label()
        save(imgdir, "nlayers_by_track_1D")

        for i in xrange(3):
            hname = "n%i_tt" % i
            h = histos[hname]
            h.SetMarkerColor(blkrgb[i % 3]); h.SetLineColor(blkrgb[i % 3]); h.SetMarkerSize(1.3)

            if i == 0:
                ymax = 10
                h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.2)
                h.Draw()
            h.Draw("same")

        leg1.Draw()
        CMS_label()
        latex.DrawLatex(0.45, 0.82, "include trigger tower effect")
        save(imgdir, "nlayers_by_track_tt_1D")

        # TH2F
        hname = "n_2D"
        h = histos[hname]
        ymax = 10
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.2)
        draw2D([h], stats=False)
        latex.SetTextSize(0.06)
        latex.DrawLatex(0.58, 0.88, "p_{T} > 20 GeV")
        save(imgdir, "nlayers_by_track_2D")

        hname = "n_tt_2D"
        h = histos[hname]
        ymax = 10
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax * 1.2)
        draw2D([h], stats=False)
        latex.SetTextSize(0.06)
        latex.DrawLatex(0.58, 0.88, "p_{T} > 20 GeV")
        latex.SetTextSize(0.04)
        latex.DrawLatex(0.45, 0.82, "include trigger tower effect")
        save(imgdir, "nlayers_by_track_tt_2D")

        latex.SetTextSize(oldTextSize)
        donotdelete = [leg1]
        return donotdelete

    histos = bookTrack()
    projectTrack(tree, histos, nentries=nentries)
    d = drawTrack(histos, imgdir)
    print tree.GetEntries(), histos.items()[0][1].GetEntries()


# ______________________________________________________________________________
if sections["fixme"]:

    pass
