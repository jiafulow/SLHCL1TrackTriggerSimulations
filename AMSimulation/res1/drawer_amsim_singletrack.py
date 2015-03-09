#!/usr/bin/env python

from rootdrawing import *
from parser import *
from helper import *
import itertools

# Configurations
colors1 = blkrgb[:3]
colors2 = blkrgb[:1] + blkrgb[3:]

# Load trigger tower map
ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)

donotdelete = []  # persist in memory


# ______________________________________________________________________________
def bookCoverage():
    histos = {}

    prefix = "coverage_"  # coverage is efficiency in the fiducial region
    ytitle = "coverage"
    for i in xrange(3):
        hname = (prefix + "pt_%i") % i
        histos[hname]       = TProfile(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 200., 0., 2., "")

        hname = (prefix + "ppt_%i") % i  # pt, but zoomed in to 0-5 GeV
        histos[hname]       = TProfile(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 5., 0., 2., "")

        hname = (prefix + "eta_%i") % i
        histos[hname]       = TProfile(hname, "; #eta; %s" % ytitle,        120, -3.0, 3.0, 0., 2., "")

        hname = (prefix + "phi_%i") % i
        histos[hname]       = TProfile(hname, "; #phi; %s" % ytitle,        128, -3.2, 3.2, 0., 2., "")

        hname = (prefix + "vz_%i") % i
        histos[hname]       = TProfile(hname, "; v_{z} [cm]; %s" % ytitle,  120, -30, 30, 0., 2., "")

        hname = (prefix + "charge_%i") % i
        histos[hname]       = TProfile(hname, "; charge; %s" % ytitle,      5, -2.5, 2.5, 0., 2., "")

    prefix = "efficiency_"  # coverage is efficiency in the fiducial region
    ytitle = "#varepsilon_{AM PR}"     # including acceptance effect
    for i in xrange(3):
        hname = (prefix + "pt_%i") % i
        histos[hname]       = TProfile(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 200., 0., 2., "")

        hname = (prefix + "ppt_%i") % i  # pt, but zoomed in to 0-5 GeV
        histos[hname]       = TProfile(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 5., 0., 2., "")

        hname = (prefix + "eta_%i") % i
        histos[hname]       = TProfile(hname, "; #eta; %s" % ytitle,        120, -3.0, 3.0, 0., 2., "")

        hname = (prefix + "phi_%i") % i
        histos[hname]       = TProfile(hname, "; #phi; %s" % ytitle,        128, -3.2, 3.2, 0., 2., "")

        hname = (prefix + "vz_%i") % i
        histos[hname]       = TProfile(hname, "; v_{z} [cm]; %s" % ytitle,  120, -30, 30, 0., 2., "")

        hname = (prefix + "charge_%i") % i
        histos[hname]       = TProfile(hname, "; charge; %s" % ytitle,      5, -2.5, 2.5, 0., 2., "")

    # Style
    for k, v in histos.iteritems():
        v.SetLineWidth(2); v.SetFillStyle(0)
        v.SetMarkerStyle(24); v.SetMarkerSize(0.9)

        hname = v.GetName()
        i = int(hname[-1])
        hname = hname[:-2]
        if hname.endswith("pt"):
            v.SetMarkerColor(colors1[i % 3]); v.SetLineColor(colors1[i % 3])
        else:
            v.SetMarkerColor(colors2[i % 3]); v.SetLineColor(colors2[i % 3])
    donotdelete.append(histos)
    return histos


def projectCoverage(tree, histos, options):
    # Merge barrel and endcap layers
    layermap = {
        5:0, 6:1, 7:2, 8:3, 9:4, 10:5,
        11:5, 12:4, 13:3, 14:2, 15:1,
        18:5, 19:4, 20:3, 21:2, 22:1,
    }

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
            prefix = "coverage_"
            if abs(eta) < 2.2/3:
                histos[prefix + "pt_0"    ].Fill(pt      , trigger)
                histos[prefix + "ppt_0"   ].Fill(pt      , trigger)
            elif abs(eta) < 4.4/3:
                histos[prefix + "pt_1"    ].Fill(pt      , trigger)
                histos[prefix + "ppt_1"   ].Fill(pt      , trigger)
            elif abs(eta) < 6.6/3:
                histos[prefix + "pt_2"    ].Fill(pt      , trigger)
                histos[prefix + "ppt_2"   ].Fill(pt      , trigger)

            if pt >= 20:
                histos[prefix + "eta_0"   ].Fill(eta     , trigger)
                histos[prefix + "phi_0"   ].Fill(phi     , trigger)
                histos[prefix + "vz_0"    ].Fill(vz      , trigger)
                histos[prefix + "charge_0"].Fill(charge  , trigger)
            elif pt >= 5:
                histos[prefix + "eta_1"   ].Fill(eta     , trigger)
                histos[prefix + "phi_1"   ].Fill(phi     , trigger)
                histos[prefix + "vz_1"    ].Fill(vz      , trigger)
                histos[prefix + "charge_1"].Fill(charge  , trigger)
            elif pt >= 2:
                histos[prefix + "eta_2"   ].Fill(eta     , trigger)
                histos[prefix + "phi_2"   ].Fill(phi     , trigger)
                histos[prefix + "vz_2"    ].Fill(vz      , trigger)
                histos[prefix + "charge_2"].Fill(charge  , trigger)

        prefix = "efficiency_"
        if abs(eta) < 2.2/3:
            histos[prefix + "pt_0"    ].Fill(pt      , trigger)
            histos[prefix + "ppt_0"   ].Fill(pt      , trigger)
        elif abs(eta) < 4.4/3:
            histos[prefix + "pt_1"    ].Fill(pt      , trigger)
            histos[prefix + "ppt_1"   ].Fill(pt      , trigger)
        elif abs(eta) < 6.6/3:
            histos[prefix + "pt_2"    ].Fill(pt      , trigger)
            histos[prefix + "ppt_2"   ].Fill(pt      , trigger)

        if pt >= 20:
            histos[prefix + "eta_0"   ].Fill(eta     , trigger)
            histos[prefix + "phi_0"   ].Fill(phi     , trigger)
            histos[prefix + "vz_0"    ].Fill(vz      , trigger)
            histos[prefix + "charge_0"].Fill(charge  , trigger)
        elif pt >= 5:
            histos[prefix + "eta_1"   ].Fill(eta     , trigger)
            histos[prefix + "phi_1"   ].Fill(phi     , trigger)
            histos[prefix + "vz_1"    ].Fill(vz      , trigger)
            histos[prefix + "charge_1"].Fill(charge  , trigger)
        elif pt >= 2:
            histos[prefix + "eta_2"   ].Fill(eta     , trigger)
            histos[prefix + "phi_2"   ].Fill(phi     , trigger)
            histos[prefix + "vz_2"    ].Fill(vz      , trigger)
            histos[prefix + "charge_2"].Fill(charge  , trigger)


    # Loop over events
    for ievt, evt in enumerate(tree):
        if (ievt == options.nentries):  break

        # Get track info
        pt     = evt.genParts_pt[0]
        eta    = evt.genParts_eta[0]
        phi    = evt.genParts_phi[0]
        vz     = evt.genParts_vz[0]
        charge = evt.genParts_charge[0]

        # Apply trigger tower requirement
        if not (options.etamin < eta < options.etamax and options.phimin < phi < options.phimax):
            continue

        # Get trigger results
        trigger = False
        for bankIndex in evt.AMTTRoads_bankIndex:
            if bankIndex < options.npatterns:
                trigger = True

        # Loop over stub moduleIds
        moduleIds_by_mlayers = [[], [], [], [], [], []]  # 6 mlayers
        for moduleId in evt.TTStubs_modId:
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
                    # Apply trigger tower requirement
                    if not (tt == options.tower):
                        continue
                    if count == 6:
                        accept = True
                        break
                if accept:
                    break

        # Debug
        #print ievt
        #print "..", pt, eta, phi, vz, charge, accept, trigger
        #for moduleId in evt.TTStubs_modId:
        #    print "....", moduleId, ttrmap[moduleId]

        fill(pt, eta, phi, vz, charge, accept, trigger)

    tree.SetBranchStatus("*", 1)
    return


def drawCoverage(histos, options, hnlambda):
    def parse_ss(ss):
        if "lu" in ss:
            ss = ss.replace("lu", "")
        elif "sf" in ss:
            ss = ss.replace("sf", "sf=").replace("_nz", ",nz=")
            ss = ss.replace("0p", "0.")
        ss = ss.replace("_", " ")
        return ss

    for hvar in ["pt", "ppt", "eta", "phi", "vz", "charge"]:
        ymax = 1.2

        hname = "%s_%i" % (hvar,0)
        h = histos[hnlambda(hname)]
        h.SetStats(0); h.SetMinimum(0); h.SetMaximum(ymax)
        h.Draw()

        # Reference lines for 0.4 and 1.0
        xmin, xmax = h.GetXaxis().GetXmin(), h.GetXaxis().GetXmax()
        tline.DrawLine(xmin, 1.0, xmax, 1.0)
        tline.DrawLine(xmin, 0.4, xmax, 0.4)
        for i in xrange(3):
            hname1 = "%s_%i" % (hvar,i)
            histos[hnlambda(hname1)].Draw("same")

        # Out of sample range
        tboxes = []
        if hvar == "pt" or hvar == "ppt":
            tboxes = [
                TBox(xmin, 0, 2.0, ymax),
                TBox(min(xmax,200.0), 0, xmax, ymax),
            ]

        # Apply trigger tower requirement
        if hvar == "eta":
            tboxes = [
                TBox(xmin, 0, options.etamin, ymax),
                TBox(options.etamax, 0, xmax, ymax),
            ]
        elif hvar == "phi":
            tboxes = [
                TBox(xmin, 0, options.phimin, ymax),
                TBox(options.phimax, 0, xmax, ymax),
            ]

        for box in tboxes:
            box.SetFillStyle(3004); box.SetFillColor(kGray)
            box.Draw()

        # Legend
        moveLegend(0.48,0.22,0.94,0.34); tlegend.Clear()
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
            tlegend.AddEntry(histos[hnlambda(hname1)], writeme[i], "lp")
        tlegend.Draw()

        tlatex.DrawLatex(0.6, 0.185, "%s [%.2fM bank]" % (parse_ss(options.ss), options.npatterns*1e-6))
        CMS_label()
        save(options.outdir, "%s_%s" % (hnlambda(hname), options.ss), dot_root=True)

    donotdelete.append(tboxes)
    return


def sitrepCoverage(histos, options):
    print "--- SITREP ---------------------------------------------------------"
    print "--- Using tt{0}, pu{1}, ss={2}, npatterns={3}, coverage={4}".format(options.tower, options.pu, options.ss, options.npatterns, options.coverage)
    print "eta=[{1:.3f},{2:.3f}], phi=[{3:.3f},{4:.3f}]".format(options.tower,
          options.etamin, options.etamax, options.phimin, options.phimax)


# ______________________________________________________________________________
# Main function
def main(options):

    # Init
    drawerInit = DrawerInit()
    tchain = TChain("ntupler/tree", "")
    tchain.AddFileInfoList(options.tfilecoll.GetList())

    # Process
    histos = bookCoverage()
    projectCoverage(tchain, histos, options)
    drawCoverage(histos, options, lambda x: "coverage_"+x)
    drawCoverage(histos, options, lambda x: "efficiency_"+x)
    sitrepCoverage(histos,options)


# ______________________________________________________________________________
if __name__ == '__main__':

    # Setup argument parser
    parser = argparse.ArgumentParser()

    # Add default arguments
    add_drawer_arguments(parser)

    # Add more arguments
    parser.add_argument("ss", help="short name of superstrip definition (e.g. ss256)")
    parser.add_argument("npatterns", type=int, help="number of patterns to reach the desired coverage")
    parser.add_argument("--coverage", type=float, default=0.9, help="desired coverage (default: %(default)s)")

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)
    options.pu = 0  # assume zero pileup

    # Calculate trigger tower fiducial region
    if options.tower != 99:
        ieta = options.tower/8
        iphi = options.tower%8
        options.etamin = -2.2 + (4.4/6) * ieta
        options.etamax = -2.2 + (4.4/6) * (ieta+1)
        if iphi < 6:
            options.phimin = -pi/2 + (2*pi/8) * iphi
            options.phimax = -pi/2 + (2*pi/8) * (iphi+1)
        else:
            options.phimin = -2*pi -pi/2 + (2*pi/8) * iphi
            options.phimax = -2*pi -pi/2 + (2*pi/8) * (iphi+1)

    # Call the main function
    main(options)
