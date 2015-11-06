#!/usr/bin/env python

from rootdrawing import *
from parser import *
from math import sinh

col  = TColor.GetColor("#1f78b4")  # mu0
fcol = TColor.GetColor("#a6cee3")  # mu0

# ______________________________________________________________________________
parnames = ["#phi", "cot #theta", "z_{0} [cm]", "1/p_{T} [1/GeV]"]
nvariables = 12
nparameters = 4


# ______________________________________________________________________________
def drawer_book():
    histos = {}

    for i in xrange(nvariables):
        hname = "npc%i" % i
        histos[hname] = TH1F(hname, "; norm pc %i" % i, 1000, -10., 10.)

    for i in xrange(nparameters):
        hname = "par%i" % i
        histos[hname] = TH1F(hname, "; gen %s" % parnames[i], 1000, -1., 1.)

        hname = "fitpar%i" % i
        histos[hname] = TH1F(hname, "; fit %s" % parnames[i], 1000, -1., 1.)

        hname = "errpar%i" % i
        histos[hname] = TH1F(hname, "; #Delta%s" % parnames[i], 1000, -0.1, 0.1)

    hname = "chi2Red"
    histos[hname] = TH1F(hname, "; #chi^{2}/ndof", 1000, 0, 10.)

    # Change binning
    histos["par0"]   .SetBins(1000, 0.6, 1.8)
    histos["par1"]   .SetBins(1000, -0.1, 0.9)
    histos["par2"]   .SetBins(1000, -25., 25.)
    histos["par3"]   .SetBins(1000, -0.6, 0.6)

    histos["fitpar0"].SetBins(1000, 0.6, 1.8)
    histos["fitpar1"].SetBins(1000, -0.1, 0.9)
    histos["fitpar2"].SetBins(1000, -25., 25.)
    histos["fitpar3"].SetBins(1000, -0.6, 0.6)

    histos["errpar0"].SetBins(1000, -0.005, 0.005)
    histos["errpar1"].SetBins(1000, -0.02, 0.02)
    histos["errpar2"].SetBins(1000, -0.5, 0.5)
    histos["errpar3"].SetBins(1000, -0.02, 0.02)

    # Style
    for hname, h in histos.iteritems():
        h.SetLineWidth(2); h.SetMarkerSize(0)
        h.SetLineColor(col); h.SetFillColor(fcol)
        h.SetFillStyle(0)
    donotdelete.append(histos)
    return histos

def parse_parname(s):
    s = s.replace("[1/GeV]","")
    s = s.replace("[GeV]","")
    s = s.replace("[cm]","")
    s = s.replace("#","")
    s = s.replace(" ","")
    s = s.replace("_","")
    s = s.replace("{","")
    s = s.replace("}","")
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
    tree.SetBranchStatus("AMTTTracks_principals", 1)

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
        #gen_pt       = float(charge) * pt
        #gen_eta      = eta


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
        hname = "chi2Red"
        histos[hname].Fill(minChi2Red)

        if options.verbose:  print ievt, itrack, "/", ntracks, minChi2Red

        if ntracks and minChi2Red != 999999. and minChi2Red < options.maxChi2:
            phi0       = evt.AMTTTracks_phi0      [itrack]
            cottheta   = evt.AMTTTracks_cottheta  [itrack]
            z0         = evt.AMTTTracks_z0        [itrack]
            invPt      = evt.AMTTTracks_invPt     [itrack]
            principals = evt.AMTTTracks_principals[itrack]

            assert(principals.size() == nvariables)

            for i in xrange(nvariables):
                hname = "npc%i" % i
                histos[hname].Fill(principals[i])

            pairs = [(gen_phi0, phi0), (gen_cottheta, cottheta), (gen_z0, z0), (gen_invPt, invPt)]
            for i in xrange(len(pairs)):
                hname = "par%i" % i
                histos[hname].Fill(pairs[i][0])

                hname = "fitpar%i" % i
                histos[hname].Fill(pairs[i][1])

                hname = "errpar%i" % i
                histos[hname].Fill(pairs[i][1]-pairs[i][0])

                if options.verbose:  print ".. {0:9} {1:15f} {2:15f} {3:15f}".format(parse_parname(parnames[i]), pairs[i][0], pairs[i][1], pairs[i][1]-pairs[i][0])

    tree.SetBranchStatus("*", 1)
    return

def drawer_draw(histos, options):
    def displayQuantiles(h, in_quantiles=[0.95,0.99], scalebox=(1.,1.)):
        # Display one-sided confidence intervals, a.k.a quantiles
        n = len(in_quantiles)
        in_quantiles = array('d', in_quantiles)
        quantiles = array('d', [0. for i in xrange(n)])
        h.GetQuantiles(n, quantiles, in_quantiles)

        gPad.Modified(); gPad.Update()
        ps = h.FindObject("stats")
        ps.SetName("mystats")

        newX1NDC = ps.GetX2NDC() - (ps.GetX2NDC() - ps.GetX1NDC()) * scalebox[0]
        newY1NDC = ps.GetY2NDC() - ((ps.GetY2NDC() - ps.GetY1NDC()) / 5 * (5 + n)) * scalebox[1]
        ps.SetX1NDC(newX1NDC)
        ps.SetY1NDC(newY1NDC)

        for iq, q in enumerate(in_quantiles):
            ps.AddText("%i%% CI = %6.4g" % (int(q*100), quantiles[iq]))
        h.stats = [h.GetMean()] + quantiles.tolist()

        h.SetStats(0)
        #gPad.Modified(); gPad.Update()
        ps.Draw()

    for hname, h in histos.iteritems():
        if options.logy:
            h.SetMaximum(h.GetMaximum() * 14); h.SetMinimum(0.5)
        else:
            h.SetMaximum(h.GetMaximum() * 1.4); h.SetMinimum(0.)

        h.Draw("hist")
        gPad.SetLogy(options.logy)

        if "errpar" in hname:
            h.Fit("gaus", "N")

        if hname == "chi2Red":
            displayQuantiles(h)

        CMS_label()
        save(options.outdir, hname)
    return

def drawer_sitrep(histos, options):
    print "--- SITREP ---------------------------------------------------------"

    for i in xrange(nparameters):
        print "{0:9} {1:15f} {2:15f}".format(parse_parname(parnames[i]), histos["errpar%i" % i].GetMean(), histos["errpar%i" % i].GetRMS())


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
