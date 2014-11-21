#!/usr/bin/env python

from rootdrawing import *
from roothelper import *

# ______________________________________________________________________________
# Configurations

sections = {}
sections["residual"] = True


drawerInit = DrawerInit()

infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/stubs_barrel_400M/stubs_barrel_400M.1.root"]
#infiles = ["/eos/uscms/store/user/l1upgrades/SLHC/GEN/620_SLHC12p1_ntuple/stubs_barrel_400M/stubs_barrel_400M.2.root"]

col  = TColor.GetColor("#1f78b4")  # Paired
fcol = TColor.GetColor("#a6cee3")
imgdir = "figures_stubs/"


# Number of events
nentries = 500000
#nentries = 5000

chain = TChain("ntupler/tree", "")
for f in infiles:
    chain.Add(f)
tree = chain

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)


# ______________________________________________________________________________
if sections["residual"]:

    def doit(x, imgdir=None, logy=False):
        histos = book(x)
        project(tree, histos, nentries=nentries, drawOverflow=False, drawUnderflow=False)
        draw(histos, logy=logy)
        save(imgdir, x[0].name)
        return histos


    # Exactly 6 stubs
    cut = "@TTStubs_r.size()==6"

    # Barrel
    phi_xmaxs = [0.006, 0.008, 0.010, 0.014, 0.018, 0.024]
    z_xmaxs = [0.5,0.5,0.5,5,5,5]
    for i in xrange(5,11):
        layer = "b%i" % i
        j = i-5
        var = "TTStubs_phi[%i] - (genParts_phi[0] - 0.3*3.8*TTStubs_r[%i]*1e-2 * (genParts_charge[0]/genParts_pt[0]) / 2.0)" % (j,j)
        xtitle = "Stub #phi_{%s} - Track #phi_{%s,exp}" % (layer,layer)
        p0 = struct("phiresidual_%s" % layer, var, cut, xtitle, 200, -phi_xmaxs[j], phi_xmaxs[j], col, fcol)
        histos = doit([p0], imgdir)
        print histos[0].h.GetRMS()

        # cot(theta) = sinh(eta) = pz/pt
        var = "TTStubs_z[%i] - (genParts_vz[0] + TTStubs_r[%i] * sinh(genParts_eta[0]))" % (j,j)
        xtitle = "Stub z_{%s} - Track z_{%s,exp} [cm]" % (layer,layer)
        p0 = struct("zresidual_%s" % layer, var, cut, xtitle, 200, -z_xmaxs[j], z_xmaxs[j], col, fcol)
        histos = doit([p0], imgdir)
        print histos[0].h.GetRMS()
