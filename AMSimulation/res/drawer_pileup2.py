#!/usr/bin/env python

from rootdrawing import *
from roothelper import *
from math import atan2

# ______________________________________________________________________________
# Configurations

samples = {}
samples["nu140"        ] = False
samples["tt140"        ] = False
samples["tttt140"      ] = True

sections = {}
sections["trkParts"    ] = False
sections["occupancy"   ] = True
sections["distribution"] = False

drawerInit = DrawerInit()
gStyle.SetNumberContours(100)
gStyle.SetPalette(55)  # rainbow color map

# Muon gun
infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1/SingleMuMinus_Barrel_20141105/SingleMu_Barrel_E2023TTI_ntuple_1_1_qdn.root"]

col  = TColor.GetColor("#1f78b4")  # Paired
fcol = TColor.GetColor("#a6cee3")
imgdir = "figures_pileup2/"

# Neutrino gun (140PU)
if samples["nu140"]:
    infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/PU140/Neutrino_E2023TTI_PU140_TuneCUEP6S1_ntuple.2.root"]
    col  = TColor.GetColor("#e31a1c")  # Paired
    fcol = TColor.GetColor("#fb9a99")
    imgdir = "figures_pileup2_nu140/"

# ttbar (140PU)
if samples["tt140"]:
    infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/PU140/TTbar_E2023TTI_PU140_TuneCUEP6S1_ntuple.2.root"]
    col  = TColor.GetColor("#6a3d9a")  # Paired
    fcol = TColor.GetColor("#cab2d6")
    imgdir = "figures_pileup2_tt140/"

# 4tops (140PU)
if samples["tttt140"]:
    infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/PU140/TTbarTTbar_E2023TTI_PU140_TuneCUEP6S1_ntuple.2.root"]
    col  = TColor.GetColor("#6a3d9a")  # Paired
    fcol = TColor.GetColor("#cab2d6")
    imgdir = "figures_pileup2_tttt140/"

# Number of events
nentries = 1000
#nentries = 20

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

coordmap = json.load(open("../data/module_coordinates.json"), object_pairs_hook=convert_key_to_int)

vertexmap = json.load(open("../data/module_vertices.json"), object_pairs_hook=convert_key_to_int)


if sections["trkParts"]:

    def doit(x, imgdir=None, logy=False):
        histos = book(x)
        project(tree, histos, nentries=nentries, drawOverflow=False, drawUnderflow=False)
        draw(histos, logy=logy)
        save(imgdir, x[0].name)
        return histos

    cut = "(trkParts_primary) * (0.785398 <= trkParts_phi && trkParts_phi <= 1.57079) * (0 <= trkParts_eta && trkParts_eta <= 0.733333)"

    writehistos = []

    p0 = struct("trkParts_size", "Sum$(%s)" % cut, "", "# trkParts", 1000, 0, 1000, col, fcol)
    histos = doit([p0], imgdir, logy=True)
    histos[0].h.SetName(p0.name); writehistos.append(histos[0].h)

    p0 = struct("trkParts_size_pt2", "Sum$(%s * %s)" % (cut, "(trkParts_pt > 2)"), "", "# trkPartsi (p_{T}>2)", 1000, 0, 1000, col, fcol)
    histos = doit([p0], imgdir, logy=True)
    histos[0].h.SetName(p0.name); writehistos.append(histos[0].h)

    p0 = struct("trkParts_size_pt3", "Sum$(%s * %s)" % (cut, "(trkParts_pt > 3)"), "", "# trkPartsi (p_{T}>3)", 1000, 0, 1000, col, fcol)
    histos = doit([p0], imgdir, logy=True)
    histos[0].h.SetName(p0.name); writehistos.append(histos[0].h)

    p0 = struct("trkParts_pt", "trkParts_pt", cut, "trkPart p_{T} [GeV]", 1000, 0., 200, col, fcol)
    histos = doit([p0], imgdir, logy=True)
    histos[0].h.SetName(p0.name); writehistos.append(histos[0].h)

    p0 = struct("trkParts_eta", "trkParts_eta", cut, "trkPart #eta", 1200, -3.0, 3.0, col, fcol)
    histos = doit([p0], imgdir, logy=True)
    histos[0].h.SetName(p0.name); writehistos.append(histos[0].h)

    p0 = struct("trkParts_phi", "trkParts_phi", cut, "trkPart #phi", 1280, -3.2, 3.2, col, fcol)
    histos = doit([p0], imgdir, logy=True)
    histos[0].h.SetName(p0.name); writehistos.append(histos[0].h)

    f = TFile.Open(imgdir+"trkParts_histos.root", "RECREATE")
    for h in writehistos:
        h.Write()
    f.Close()


# ______________________________________________________________________________
if sections["occupancy"]:

    # if ROOT complains about "Failed to load Dynamic link library",
    # trash the existing .so file.
    gROOT.ProcessLine(".L ../test/superstrip_rational.C+O")
    from ROOT import superstrip_rational

    gStyle.SetLabelSize(0.04, "XY")
    latex.SetTextSize(0.04)

    # from http://stackoverflow.com/questions/480214/how-do-you-remove-duplicates-from-a-list-in-python-whilst-preserving-order
    def unique_f7(seq):
        seen = set()
        seen_add = seen.add
        return [x for x in seq if not (x in seen or seen_add(x))]


    def bookLayer():
        histos = {}
        # Use correct bin centers to get correct means
        nbins, xmin, xmax = 200, 0, 200
        for i in xrange(5,11):
            hname = "nstubs_b%i" % i
            histos[hname] = TH1F(hname, "; # stubs/layer/tower/BX; Entries", nbins+1, xmin-0.5, xmax+0.5)

            hname = "nsuperstrips_b%i" % i
            histos[hname] = TH1F(hname, "; # superstrips/layer/tower/BX; Entries", nbins+1, xmin-0.5, xmax+0.5)

            hname = "nstubs_per_superstrip_b%i" % i
            histos[hname] = TH1F(hname, "; # stubs/superstrip/layer/tower/BX; Entries", nbins/10+1, xmin-0.5, xmax/10+0.5)

            hname = "nmodules_b%i" % i
            histos[hname] = TH1F(hname, "; # modules/layer/tower/BX; Entries", nbins/2+1, xmin-0.5, xmax/2+0.5)

            hname = "nstubs_per_module_b%i" % i
            histos[hname] = TH1F(hname, "; # stubs/module/layer/tower/BX; Entries", nbins/2+1, xmin-0.5, xmax/2+0.5)

        for k, v in histos.iteritems():
            v.SetLineColor(col); v.SetLineWidth(2); v.SetFillColor(fcol)
        return histos

    def projectLayer(tree, histos, tt, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("TTStubs_phi"           , 1)
        tree.SetBranchStatus("TTStubs_z"             , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        superstripIds_all = []
        phimins, phimaxs = [], []
        zmins, zmaxs = [], []
        for i in xrange(5,11):
            superstripIds_all.append(set())
            phimins.append(9999.); phimaxs.append(-9999.)
            zmins.append(9999.); zmaxs.append(-9999.)

        for moduleId in ttmap[tt]:
            c = vertexmap[moduleId]
            assert(c[0] == c[3*3] and c[0+1] == c[3*3+1] and c[3] == c[3*2] and c[3+1] == c[3*2+1])
            assert(c[0+2] == c[3+2] and c[3*2+2] == c[3*3+2])
            phi0 = atan2(c[0+1], c[0])
            phi1 = atan2(c[3+1], c[3])
            z0 = c[2]
            z1 = c[3*2+2]
            if phi1 < phi0:  phi0, phi1 = phi1, phi0
            if z1 < z0: z0, z1 = z1, z0

            lay = decodeLayer(moduleId)
            ilay = lay-5
            for j in xrange(1024):
                phi = phi0 + (phi1 - phi0)/1024. * j
                ss = superstrip_rational(ilay, phi, 0., 1.0)
                superstripIds_all[ilay].add(ss)
                if phimins[ilay] > phi:  phimins[ilay] = phi
                if phimaxs[ilay] < phi:  phimaxs[ilay] = phi
            if zmins[ilay] > z0: zmins[ilay] = z0
            if zmaxs[ilay] < z1: zmaxs[ilay] = z1

        for i in xrange(5,11):
            ii = i-5
            print "b%i" % i, len(superstripIds_all[ii]), "phi=[%f,%f]" % (phimins[ii], phimaxs[ii]), "z=[%f,%f]" % (zmins[ii], zmaxs[ii])

        groups = []

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            for i in xrange(5,11):
                groups.append([])  # init groups

            # Loop over stubs
            for phi, z, moduleId in izip(ievt.TTStubs_phi, ievt.TTStubs_z, ievt.TTStubs_modId):
                if tt in ttrmap.get(moduleId, []):
                    lay = decodeLayer(moduleId)
                    ilay = lay-5
                    ss = superstrip_rational(ilay, phi, 0., 1.0)
                    groups[ilay].append((moduleId,ss))

                    if phimins[ilay] > phi or phimaxs[ilay] < phi:
                        print "WARNING: phi not in b%i range: %f" % (lay,phi)
                    if zmins[ilay] > z or zmaxs[ilay] < z:
                        print "WARNING: z not in b%i range: %f" % (lay,phi)

            for i in xrange(5,11):
                ii = i-5
                nstubs = len(groups[ii])

                moduleIds = {}
                superstripIds = {}
                for moduleId, ss in groups[ii]:
                    moduleIds[moduleId] = moduleIds.get(moduleId, 0) + 1
                    superstripIds[ss] = superstripIds.get(ss, 0) + 1

                histos["nstubs_b%i" % i].Fill(nstubs)

                histos["nmodules_b%i" % i].Fill(len(moduleIds))
                for moduleId in ttmap[tt]:
                    histos["nstubs_per_module_b%i" % i].Fill(moduleIds.get(moduleId, 0))

                histos["nsuperstrips_b%i" % i].Fill(len(superstripIds))

                for ss in superstripIds_all[ii]:
                    histos["nstubs_per_superstrip_b%i" % i].Fill(superstripIds.get(ss, 0))

            del groups[:]  # clear groups

        tree.SetBranchStatus("*", 0)
        return

    def drawLayer(histos, tt, hnlambda, imgdir, logy=False):
        ymax, ymin = -1, 0
        for i in xrange(5,11):
            hname = "nstubs_b%i" % i
            h = histos[hnlambda(hname)]
            if ymax == -1:
                if logy:
                    ymax = h.GetMaximum() * 5.0
                    ymin = 0.5
                else:
                    ymax = h.GetMaximum() * 1.5
                    ymin = 0

            h.SetMaximum(ymax); h.SetMinimum(ymin)
            h.Draw("hist")
            if logy: gPad.SetLogy(1)
            latex.DrawLatex(0.7, 0.7, "#color[%i]{tt%i, b%i}" % (col,tt,i))
            CMS_label()
            save(imgdir, "occupancy_%s" % hnlambda(hname))
            if logy: gPad.SetLogy(0)

        donotdelete = []
        return donotdelete

    tt = 27
    histos = bookLayer()
    projectLayer(tree, histos, tt, nentries=nentries)
    d = drawLayer(histos, tt, lambda x: x, imgdir)
    d = drawLayer(histos, tt, lambda x: "nsuperstrips_"+x[7:], imgdir)
    d = drawLayer(histos, tt, lambda x: "nstubs_per_superstrip_"+x[7:], imgdir, logy=True)
    d = drawLayer(histos, tt, lambda x: "nmodules_"+x[7:], imgdir)
    d = drawLayer(histos, tt, lambda x: "nstubs_per_module_"+x[7:], imgdir, logy=True)


# ______________________________________________________________________________
if sections["distribution"]:

    gStyle.SetLabelSize(0.04, "XY")
    latex.SetTextSize(0.04)

    rmeans = b_rcoord_cmssw
    zmeans = e_zcoord_cmssw

    rmins = []
    zmins = []
    for i in xrange(5,16):
        if i < 11:
            rmins.append(set())
        elif i < 16:
            zmins.append(set())
    for k, v in coordmap.iteritems():
        lay = decodeLayer(k)
        if lay < 11:
            ilay = lay-5
            rmins[ilay].add(v[1])
        elif lay < 16:
            ilay = lay-11
            zmins[ilay].add(v[0])
        elif lay < 18:
            continue
        else:
            ilay = lay - 18
            zmins[ilay].add(-v[0])
    rmins = [sorted(x) for x in rmins]
    zmins = [sorted(x) for x in zmins]
    for i in xrange(5,16):
        if i == 11 or i == 18:
            print
        if i < 11:
            ii = i-5
            print "b%i" % i, "r=[%s]" % (",".join(map(str,rmins[ii]))), "mean r=%f" % (sum(rmins[ii])/len(rmins[ii]))
        elif i < 16:
            ii = i-11
            print "e%i" % i, "z=[%s]" % (",".join(map(str,zmins[ii]))), "mean z=%f" % (sum(zmins[ii])/len(zmins[ii]))


    def bookLayer():
        histos = {}
        for i in xrange(5,11):
            hname = "phi_b%i" % i
            histos[hname] = TH1F(hname, "; stub #phi; Entries", 70, 0.5, 1.9)

            hname = "eta_b%i" % i
            histos[hname] = TH1F(hname, "; stub #eta; Entries", 75, -0.4, 1.1)

            hname = "r_b%i" % i
            r = rmeans[i-5]
            histos[hname] = TH1F(hname, "; stub r [cm]; Entries", 250, r-2.5, r+2.5)

            hname = "z_b%i" % i
            histos[hname] = TH1F(hname, "; stub z [cm]; Entries", 75, -40, 110)

        for k, v in histos.iteritems():
            v.SetLineColor(col); v.SetLineWidth(2); v.SetFillColor(fcol)
        return histos

    def projectLayer(tree, histos, tt, nentries=1000000000):
        tree.SetBranchStatus("*", 0)
        tree.SetBranchStatus("TTStubs_r"             , 1)
        tree.SetBranchStatus("TTStubs_phi"           , 1)
        tree.SetBranchStatus("TTStubs_z"             , 1)
        tree.SetBranchStatus("TTStubs_eta"           , 1)
        tree.SetBranchStatus("TTStubs_modId"         , 1)

        # Loop over events
        for i_ievt, ievt in enumerate(tree):
            if (i_ievt == nentries):  break

            # Loop over stubs
            for r, phi, z, eta, moduleId in izip(ievt.TTStubs_r, ievt.TTStubs_phi, ievt.TTStubs_z, ievt.TTStubs_eta, ievt.TTStubs_modId):
                if tt in ttrmap.get(moduleId, []):
                    lay = decodeLayer(moduleId)
                    ilay = lay-5

                    hname = "phi_b%i" % lay
                    histos[hname].Fill(phi)

                    hname = "eta_b%i" % lay
                    histos[hname].Fill(eta)

                    hname = "r_b%i" % lay
                    histos[hname].Fill(r)

                    hname = "z_b%i" % lay
                    histos[hname].Fill(z)

                    if round(r,4) < rmins[ilay][0]:
                        print "WARNING: r not in b%i range: %f" % (lay,r)

        tree.SetBranchStatus("*", 0)
        return

    def drawLayer(histos, tt, hnlambda, imgdir, logy=False):
        ymax = -1
        for i in xrange(5,11):
            hname = "phi_b%i" % i
            h = histos[hnlambda(hname)]
            if ymax == -1:
                ymax = h.GetMaximum() * 1.5

            h.SetMaximum(ymax); h.SetMinimum(0)
            h.Draw("hist")

            if hnlambda(hname).startswith("r_"):
                ii = i-5
                for x in rmins[ii]:
                    line.DrawLine(x, 0, x, ymax)
            h.Draw("same hist")

            latex.DrawLatex(0.7, 0.7, "#color[%i]{tt%i, b%i}" % (col,tt,i))
            CMS_label()
            save(imgdir, "distribution_%s" % hnlambda(hname))

        donotdelete = []
        return donotdelete

    tt = 27
    histos = bookLayer()
    projectLayer(tree, histos, tt, nentries=nentries)
    d = drawLayer(histos, tt, lambda x: x, imgdir)
    d = drawLayer(histos, tt, lambda x: "eta_"+x[4:], imgdir)
    d = drawLayer(histos, tt, lambda x: "r_"+x[4:], imgdir)
    d = drawLayer(histos, tt, lambda x: "z_"+x[4:], imgdir)
