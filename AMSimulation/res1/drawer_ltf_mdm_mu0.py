#!/usr/bin/env python

from rootdrawing import *
from parser import *
from math import sinh

col = TColor.GetColor("#008000")

# ______________________________________________________________________________
parnames = ["#phi", "cot #theta", "z_{0} [cm]", "1/p_{T} [1/GeV]"]
nvariables = 12
nparameters = 4

parameters2 = ["pt", "eta", "invPt", "z0"]
parnames2 = ["c*p_{T} [GeV]", "#eta", "1/p_{T} [1/GeV]", "z_{0} [cm]"]
nparameters2 = 4

meanmaxes = [0.001, 0.002, 0.02, 0.002]
sigmamaxes = [0.002, 0.005, 0.15, 0.005]

# pT
parnames += ["p_{T} [GeV]"]
meanmaxes += [0.005]
sigmamaxes += [0.05]

# ______________________________________________________________________________
def drawer_book():
    histos = {}

    for j in xrange(nparameters2):
        for i in xrange(nparameters+1):
            if i < nparameters:
                hname = "errpar%i_vs_%i" % (i, j)
                histos[hname] = TH2F(hname, "; %s; #Delta%s" % (parnames2[j], parnames[i]), 40, -200, 200, 1000, -0.1, 0.1)
            else:
                hname = "errpt_vs_%i" % (j)
                histos[hname] = TH2F(hname, "; %s; #Delta%s" % (parnames2[j], parnames[i]), 40, -200, 200, 1000, -0.1, 0.1)

    # Change binning
    histos["errpar0_vs_0"].SetBins(40, -200, 200, 100, -0.005, 0.005)
    histos["errpar1_vs_0"].SetBins(40, -200, 200, 100, -0.02, 0.02)
    histos["errpar2_vs_0"].SetBins(40, -200, 200, 100, -0.5, 0.5)
    histos["errpar3_vs_0"].SetBins(40, -200, 200, 100, -0.02, 0.02)
    histos["errpt_vs_0"  ].SetBins(40, -200, 200, 100, -0.20, 0.20)

    histos["errpar0_vs_1"].SetBins(40, 0, 0.8, 100, -0.005, 0.005)
    histos["errpar1_vs_1"].SetBins(40, 0, 0.8, 100, -0.02, 0.02)
    histos["errpar2_vs_1"].SetBins(40, 0, 0.8, 100, -0.5, 0.5)
    histos["errpar3_vs_1"].SetBins(40, 0, 0.8, 100, -0.02, 0.02)
    histos["errpt_vs_1"  ].SetBins(40, 0, 0.8, 100, -0.20, 0.20)

    histos["errpar0_vs_2"].SetBins(40, -0.5, 0.5, 100, -0.005, 0.005)
    histos["errpar1_vs_2"].SetBins(40, -0.5, 0.5, 100, -0.02, 0.02)
    histos["errpar2_vs_2"].SetBins(40, -0.5, 0.5, 100, -0.5, 0.5)
    histos["errpar3_vs_2"].SetBins(40, -0.5, 0.5, 100, -0.02, 0.02)
    histos["errpt_vs_2"  ].SetBins(40, -0.5, 0.5, 100, -0.20, 0.20)

    histos["errpar0_vs_3"].SetBins(40, -20, 20, 100, -0.005, 0.005)
    histos["errpar1_vs_3"].SetBins(40, -20, 20, 100, -0.02, 0.02)
    histos["errpar2_vs_3"].SetBins(40, -20, 20, 100, -0.5, 0.5)
    histos["errpar3_vs_3"].SetBins(40, -20, 20, 100, -0.02, 0.02)
    histos["errpt_vs_3"  ].SetBins(40, -20, 20, 100, -0.20, 0.20)

    # Style
    for hname, h in histos.iteritems():
        h.SetMarkerSize(1); h.SetMarkerColor(col)
    donotdelete.append(histos)
    return histos

def parse_parname_title(s, t):
    if t == "mean":
        s = "#Delta%s mean" % s
    elif t == "resolution":
        s = "%s resolution" % s
    for unit in ["[1/GeV]", "[GeV]", "[cm]"]:
        if unit in s:
            s = s.replace(" "+unit, "")
            s = s+" "+unit
    return s

def drawer_project(tree, histos, options):
    tree.SetBranchStatus("*", 0)
    tree.SetBranchStatus("genParts_pt"     , 1)
    tree.SetBranchStatus("genParts_eta"    , 1)
    tree.SetBranchStatus("genParts_phi"    , 1)
    #tree.SetBranchStatus("genParts_vx"     , 1)
    #tree.SetBranchStatus("genParts_vy"     , 1)
    tree.SetBranchStatus("genParts_vz"     , 1)
    tree.SetBranchStatus("genParts_charge" , 1)
    tree.SetBranchStatus("AMTTTracks_invPt"     , 1)
    tree.SetBranchStatus("AMTTTracks_phi0"      , 1)
    tree.SetBranchStatus("AMTTTracks_cottheta"  , 1)
    tree.SetBranchStatus("AMTTTracks_z0"        , 1)
    tree.SetBranchStatus("AMTTTracks_chi2"      , 1)
    tree.SetBranchStatus("AMTTTracks_ndof"      , 1)
    #tree.SetBranchStatus("AMTTTracks_principals", 1)

    # Loop over events
    for ievt, evt in enumerate(tree):
        if (ievt == options.nentries):  break

        nparts = evt.genParts_pt.size()
        assert(nparts == 1)

        ipart = 0
        pt      = evt.genParts_pt    [ipart]
        eta     = evt.genParts_eta   [ipart]
        phi     = evt.genParts_phi   [ipart]
        #vx      = evt.genParts_vx    [ipart]
        #vy      = evt.genParts_vy    [ipart]
        vz      = evt.genParts_vz    [ipart]
        charge  = evt.genParts_charge[ipart]

        gen_phi0     = phi
        gen_cottheta = sinh(eta)
        gen_z0       = vz
        gen_invPt    = float(charge) / pt
        gen_pt       = float(charge) * pt
        gen_eta      = eta


        ntracks = evt.AMTTTracks_ndof.size()

        minChi2Red, minChi2Red_i = 999999., 0
        for itrack in xrange(ntracks):
            chi2     = evt.AMTTTracks_chi2[itrack]
            ndof     = evt.AMTTTracks_ndof[itrack]
            chi2Red  = chi2/ndof

            if not (options.minNdof <= ndof <= options.maxNdof):
                continue

            if minChi2Red > chi2Red:
                minChi2Red = chi2Red
                minChi2Red_i = itrack

        itrack = minChi2Red_i

        if options.verbose:  print ievt, itrack, "/", ntracks, minChi2Red

        if ntracks and minChi2Red != 999999. and minChi2Red < options.maxChi2:
            phi0       = evt.AMTTTracks_phi0      [itrack]
            cottheta   = evt.AMTTTracks_cottheta  [itrack]
            z0         = evt.AMTTTracks_z0        [itrack]
            invPt      = evt.AMTTTracks_invPt     [itrack]

            pairs = [(gen_phi0, phi0), (gen_cottheta, cottheta), (gen_z0, z0), (gen_invPt, invPt)]
            pars2 = [gen_pt, gen_eta, gen_invPt, gen_z0]
            for j in xrange(nparameters2):
                for i in xrange(nparameters):
                    hname = "errpar%i_vs_%i" % (i, j)
                    histos[hname].Fill(pars2[j], pairs[i][1]-pairs[i][0])
                hname = "errpt_vs_%i" % (j)
                histos[hname].Fill(pars2[j], (abs(1.0/invPt) - abs(1.0/gen_invPt))*abs(gen_invPt))

    tree.SetBranchStatus("*", 1)
    return

def drawer_draw(histos, options):

    for j in xrange(nparameters2):
        for i in xrange(nparameters+1):
            if i < nparameters:
                hname = "errpar%i_vs_%i" % (i, j)
            else:
                hname = "errpt_vs_%i" % (j)
            h = histos[hname]
            h.Draw()

            nbinsx = h.GetNbinsX()
            gr1 = TGraphAsymmErrors(nbinsx)
            gr1.SetName(hname + "_gr1"); setattr(h, "gr1", gr1)
            gr1.SetMarkerSize(1); gr1.SetMarkerColor(col)
            gr2 = TGraphAsymmErrors(nbinsx)
            gr2.SetName(hname + "_gr2"); setattr(h, "gr2", gr2)
            gr2.SetMarkerSize(1); gr2.SetMarkerColor(col)

            for ibin in xrange(nbinsx):
                hpy = h.ProjectionY(hname + "_py%i" % ibin, ibin+1, ibin+1); setattr(h, "py%i" % ibin, hpy)
                x = h.GetBinCenter(ibin+1)

                if hpy.GetEntries() < 30:
                    print "WARNING: not enough entries in %s: %i" % (hpy.GetName(), hpy.GetEntries())
                    gr1.SetPoint(ibin, x, 999999)
                    gr2.SetPoint(ibin, x, 999999)
                    continue

                if hpy.GetEntries() < 100:
                    print "INFO: rebinning in %s: %i" % (hpy.GetName(), hpy.GetEntries())
                    hpy.Rebin(2)

                assert(hpy.GetNbinsX() > 20)
                fitxmin, fitxmax = hpy.GetBinCenter(10+1), hpy.GetBinCenter(hpy.GetNbinsX()-1-10+1)
                if ibin in [0, 21, 30, 39]:
                    hpy.Draw()
                    r = hpy.Fit("gaus", "IS", "", fitxmin, fitxmax)
                else:
                    r = hpy.Fit("gaus", "INS", "", fitxmin, fitxmax)

                p1, e1, p2, e2 = r.Parameter(1), r.ParError(1), r.Parameter(2), r.ParError(2)
                gr1.SetPoint(ibin, x, p1)
                gr1.SetPointError(ibin, 0, 0, e1, e1)
                gr2.SetPoint(ibin, x, p2)
                gr2.SetPointError(ibin, 0, 0, e2, e2)

                if ibin in [0, 21, 30, 39]:
                    print hpy.GetName(), hpy.GetEntries(), hpy.GetMean(), hpy.GetRMS(), x, p1, p2

                if options.verbose:  print "%.3g mu=%.3g+/-%.3g sigma=%.3g+/-%.3g" % (x, p1, e1, p2, e2)

            hpx1 = h.ProjectionX(hname + "_px1"); hpx1.Reset(); setattr(h, "px1", hpx1)
            if i < nparameters:
                hpx1.SetYTitle(parse_parname_title(parnames[i], "mean"))
            else:
                hpx1.SetYTitle("#Delta p_{T}/p_{T} mean")
            hpx1.SetMinimum(-meanmaxes[i]); hpx1.SetMaximum(meanmaxes[i])
            hpx1.SetStats(0); hpx1.Draw()
            gr1.Draw("p")
            CMS_label()
            save(options.outdir, "mean_"+hname)

            hpx2 = h.ProjectionX(hname + "_px2"); hpx2.Reset(); setattr(h, "px2", hpx2)
            if i < nparameters:
                hpx2.SetYTitle(parse_parname_title(parnames[i], "resolution"))
            else:
                hpx2.SetYTitle("p_{T} resolution/p_{T}")
            hpx2.SetMinimum(0); hpx2.SetMaximum(sigmamaxes[i])
            hpx2.SetStats(0); hpx2.Draw()
            gr2.Draw("p")
            CMS_label()
            save(options.outdir, "sigma_"+hname)

            for ibin in [0, 21, 30, 39]:
                hh = getattr(h, "py%i" % ibin)
                hname = hh.GetName()
                hh.SetMaximum(hh.GetMaximum() * 2)
                hh.Draw()
                CMS_label()
                save(options.outdir, "delta_"+hname)
    return

def drawer_sitrep(histos, options):
    print "--- SITREP ---------------------------------------------------------"


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
    drawer_sitrep(histos, options)


# ______________________________________________________________________________
if __name__ == '__main__':

    # Setup argument parser
    parser = argparse.ArgumentParser()

    # Add default arguments
    add_drawer_arguments(parser)

    # Add more arguments
    #parser.add_argument("ss", help="short name of superstrip definition (e.g. ss256)")
    #parser.add_argument("npatterns", type=int, help="number of patterns to reach the desired coverage")
    parser.add_argument("--maxChi2", type=float, default=5, help="max reduced chi-squared (default: %(default)s)")
    parser.add_argument("--minNdof", type=int, default=8, help="min # of degree of freedom (default: %(default)s)")
    parser.add_argument("--maxNdof", type=int, default=8, help="max # of degree of freedom (default: %(default)s)")

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)

    # Call the main function
    main(options)
