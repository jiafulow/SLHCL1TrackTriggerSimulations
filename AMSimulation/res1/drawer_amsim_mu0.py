#!/usr/bin/env python

from rootdrawing import *
from parser import *

# Configurations
colors1 = blkrgb[:3]
colors2 = blkrgb[:1] + blkrgb[3:]

maxMisses = 0
requiredLayers = [5, 6, 7, 8, 9, 10]


# ______________________________________________________________________________
# Load trigger tower map
ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)

def use_TEfficiency():
    del ROOT.TEfficiency
    gROOT.ProcessLine(".L ./fixes/TEfficiency.cxx+")
    #gSystem.Load("./fixes/TEfficiency_cxx.so")

    global TEfficiency
    TEfficiency = ROOT.TEfficiency


# ______________________________________________________________________________
def drawer_book(options):
    histos = {}

    prefix = "coverage_"  # coverage is efficiency in the fiducial region
    ytitle = "coverage"
    for i in xrange(3):
        hname = (prefix + "pt_%i") % i
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 500.)

        hname = (prefix + "ppt_%i") % i  # pt, but zoomed in to 0-50 GeV
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 50.)

        hname = (prefix + "pppt_%i") % i  # pt, but zoomed in to 0-10 GeV
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 10.)

        hname = (prefix + "eta_%i") % i
        histos[hname] = TEfficiency(hname, "; #eta; %s" % ytitle,        120, -3.0, 3.0)

        hname = (prefix + "phi_%i") % i
        histos[hname] = TEfficiency(hname, "; #phi; %s" % ytitle,        128, -3.2, 3.2)

        hname = (prefix + "vz_%i") % i
        histos[hname] = TEfficiency(hname, "; v_{z} [cm]; %s" % ytitle,  120, -30, 30)

        hname = (prefix + "charge_%i") % i
        histos[hname] = TEfficiency(hname, "; charge; %s" % ytitle,      5, -2.5, 2.5)

    prefix = "efficiency_"  # including acceptance effect
    ytitle = "#varepsilon_{AM PR}"
    for i in xrange(3):
        hname = (prefix + "pt_%i") % i
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 500.)

        hname = (prefix + "ppt_%i") % i  # pt, but zoomed in to 0-50 GeV
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 50.)

        hname = (prefix + "pppt_%i") % i  # pt, but zoomed in to 0-10 GeV
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s" % ytitle, 100, 0., 10.)

        hname = (prefix + "eta_%i") % i
        histos[hname] = TEfficiency(hname, "; #eta; %s" % ytitle,        120, -3.0, 3.0)

        hname = (prefix + "phi_%i") % i
        histos[hname] = TEfficiency(hname, "; #phi; %s" % ytitle,        128, -3.2, 3.2)

        hname = (prefix + "vz_%i") % i
        histos[hname] = TEfficiency(hname, "; v_{z} [cm]; %s" % ytitle,  120, -30, 30)

        hname = (prefix + "charge_%i") % i
        histos[hname] = TEfficiency(hname, "; charge; %s" % ytitle,      5, -2.5, 2.5)

    # Error statistics
    for hname, h in histos.iteritems():
        if h.ClassName() == "TEfficiency":
            h.SetConfidenceLevel(0.682689492137)  # 1-sigma
            h.SetStatisticOption(0)  # kFCP
            #h.SetStatisticOption(6)  # kBUniform

    # Style
    for hname, h in histos.iteritems():
        h.SetLineWidth(2); h.SetFillStyle(0)
        h.SetMarkerStyle(24); h.SetMarkerSize(1)

        i = int(hname[-1])
        hname = hname[:-2]
        if hname.endswith("pt"):
            h.SetMarkerColor(colors1[i % 3]); h.SetLineColor(colors1[i % 3])
        else:
            h.SetMarkerColor(colors2[i % 3]); h.SetLineColor(colors2[i % 3])
    donotdelete.append(histos)
    return histos


def drawer_project(tree, histos, options):
    moduleIds_tt = set()
    for moduleId in ttmap[options.tower]:
        moduleIds_tt.add(moduleId)

    tree.SetBranchStatus("*", 0)
    tree.SetBranchStatus("genParts_pt"           , 1)
    tree.SetBranchStatus("genParts_eta"          , 1)
    tree.SetBranchStatus("genParts_phi"          , 1)
    tree.SetBranchStatus("genParts_vz"           , 1)
    tree.SetBranchStatus("genParts_charge"       , 1)
    tree.SetBranchStatus("TTStubs_modId"         , 1)
    tree.SetBranchStatus("AMTTRoads_patternRef"  , 1)

    def fill(pt, eta, phi, vz, charge, accept, trigger):
        if accept:
            prefix = "coverage_"
            if abs(eta) < 2.2/3:
                histos[prefix + "pt_0"    ].Fill(trigger, pt)
                histos[prefix + "ppt_0"   ].Fill(trigger, pt)
                histos[prefix + "pppt_0"  ].Fill(trigger, pt)
            elif abs(eta) < 4.4/3:
                histos[prefix + "pt_1"    ].Fill(trigger, pt)
                histos[prefix + "ppt_1"   ].Fill(trigger, pt)
                histos[prefix + "pppt_1"  ].Fill(trigger, pt)
            elif abs(eta) < 6.6/3:
                histos[prefix + "pt_2"    ].Fill(trigger, pt)
                histos[prefix + "ppt_2"   ].Fill(trigger, pt)
                histos[prefix + "pppt_2"  ].Fill(trigger, pt)

            if pt >= 20:
                histos[prefix + "eta_0"   ].Fill(trigger, eta)
                histos[prefix + "phi_0"   ].Fill(trigger, phi)
                histos[prefix + "vz_0"    ].Fill(trigger, vz)
                histos[prefix + "charge_0"].Fill(trigger, charge)
            elif pt >= 5:
                histos[prefix + "eta_1"   ].Fill(trigger, eta)
                histos[prefix + "phi_1"   ].Fill(trigger, phi)
                histos[prefix + "vz_1"    ].Fill(trigger, vz)
                histos[prefix + "charge_1"].Fill(trigger, charge)
            elif pt >= options.ptmin:
                histos[prefix + "eta_2"   ].Fill(trigger, eta)
                histos[prefix + "phi_2"   ].Fill(trigger, phi)
                histos[prefix + "vz_2"    ].Fill(trigger, vz)
                histos[prefix + "charge_2"].Fill(trigger, charge)

        prefix = "efficiency_"
        if abs(eta) < 2.2/3:
            histos[prefix + "pt_0"    ].Fill(trigger, pt)
            histos[prefix + "ppt_0"   ].Fill(trigger, pt)
            histos[prefix + "pppt_0"  ].Fill(trigger, pt)
        elif abs(eta) < 4.4/3:
            histos[prefix + "pt_1"    ].Fill(trigger, pt)
            histos[prefix + "ppt_1"   ].Fill(trigger, pt)
            histos[prefix + "pppt_1"  ].Fill(trigger, pt)
        elif abs(eta) < 6.6/3:
            histos[prefix + "pt_2"    ].Fill(trigger, pt)
            histos[prefix + "ppt_2"   ].Fill(trigger, pt)
            histos[prefix + "pppt_2"  ].Fill(trigger, pt)

        if pt >= 20:
            histos[prefix + "eta_0"   ].Fill(trigger, eta)
            histos[prefix + "phi_0"   ].Fill(trigger, phi)
            histos[prefix + "vz_0"    ].Fill(trigger, vz)
            histos[prefix + "charge_0"].Fill(trigger, charge)
        elif pt >= 5:
            histos[prefix + "eta_1"   ].Fill(trigger, eta)
            histos[prefix + "phi_1"   ].Fill(trigger, phi)
            histos[prefix + "vz_1"    ].Fill(trigger, vz)
            histos[prefix + "charge_1"].Fill(trigger, charge)
        elif pt >= options.ptmin:
            histos[prefix + "eta_2"   ].Fill(trigger, eta)
            histos[prefix + "phi_2"   ].Fill(trigger, phi)
            histos[prefix + "vz_2"    ].Fill(trigger, vz)
            histos[prefix + "charge_2"].Fill(trigger, charge)


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

        lay_counts = {}
        for lay in requiredLayers:
            lay_counts[lay] = 0

        for istub, moduleId in enumerate(evt.TTStubs_modId):
            if moduleId not in moduleIds_tt:
                continue

            lay = decodeLayer(moduleId)
            if lay in lay_counts:
                lay_counts[lay] += 1

        accept_count = 0
        for lay in requiredLayers:
            if lay_counts[lay] > 0:
                accept_count += 1

        if accept_count >= len(requiredLayers) - maxMisses:
            accept = True
        else:
            accept = False

        # Get trigger results
        trigger = False
        for patternRef in evt.AMTTRoads_patternRef:
            if patternRef < options.npatterns:
                trigger = True

        # Debug
        #print ievt
        #print "..", pt, eta, phi, vz, charge, accept, trigger
        #for moduleId in evt.TTStubs_modId:
        #    print "....", moduleId, ttrmap[moduleId]

        fill(pt, eta, phi, vz, charge, accept, trigger)

    tree.SetBranchStatus("*", 1)
    return


def drawer_draw(histos, options, hnlambda):
    def parse_ss(ss):
        if "lu" in ss:
            ss = ss.replace("lu", "")
        elif "sf" in ss:
            ss = ss.replace("sf", "sf=").replace("_nz", ",nz=")
            ss = ss.replace("0p", "0.")
        ss = ss.replace("_", " ")
        return ss

    for hvar in ["pt", "ppt", "pppt", "eta", "phi", "vz", "charge"]:
        ymax = 1.2

        # Draw first one
        hname = "%s_%i" % (hvar,0)
        #h = histos[hnlambda(hname)]
        h = histos[hnlambda(hname)].GetCopyTotalHisto(); h.Reset()
        h.SetMinimum(0); h.SetMaximum(ymax)
        h.SetStats(0); h.Draw()

        # Reference lines for 0.9, 0.95 and 1.0
        xmin, xmax = h.GetXaxis().GetXmin(), h.GetXaxis().GetXmax()
        for y in [0.5, 0.8, 0.9, 0.95, 1.0]:
            tline.DrawLine(xmin, y, xmax, y)

        # Draw all
        for i in xrange(3):
            hname1 = "%s_%i" % (hvar,i)
            #histos[hnlambda(hname1)].Draw("same")

            histos[hnlambda(hname1)].gr = histos[hnlambda(hname1)].CreateGraph()
            histos[hnlambda(hname1)].gr.Draw("p")

        # Out of sample range
        tboxes = []
        if hvar == "pt" or hvar == "ppt" or hvar == "pppt":
            tboxes = [
                TBox(xmin, 0, options.ptmin, ymax),
                TBox(min(xmax,options.ptmax), 0, xmax, ymax),
            ]
        elif hvar == "eta":
            tboxes = [
                TBox(xmin, 0, options.etamin, ymax),
                TBox(options.etamax, 0, xmax, ymax),
            ]
        elif hvar == "phi":
            tboxes = [
                TBox(xmin, 0, options.phimin, ymax),
                TBox(options.phimax, 0, xmax, ymax),
            ]
        elif hvar == "charge":
            tboxes = [
                TBox(xmin, 0, -1.5, ymax),
                TBox(-0.5, 0, 0.5, ymax),
                TBox(1.5, 0, xmax, ymax),
            ]

        for box in tboxes:
            box.SetFillStyle(3004); box.SetFillColor(kGray)
            box.Draw()

        # Legend
        moveLegend(0.48,0.22,0.94,0.34); tlegend.Clear()
        if hvar == "pt" or hvar == "ppt" or hvar == "pppt":
            writeme = [
                "0.000 #leq |#eta| < 0.733",
                "0.733 #leq |#eta| < 1.467",
                "1.467 #leq |#eta| < 2.200",
            ]
            for i in xrange(1):
                hname1 = "%s_%i" % (hvar,i)
                tlegend.AddEntry(histos[hnlambda(hname1)], writeme[i], "lp")
        else:
            writeme = [
                "20 #leq p_{T} < #infty  GeV",
                "  5 #leq p_{T} < 20 GeV",
                "  %.0f #leq p_{T} <   5 GeV" % options.ptmin,
            ]
            for i in xrange(3):
                hname1 = "%s_%i" % (hvar,i)
                tlegend.AddEntry(histos[hnlambda(hname1)], writeme[i], "lp")
        tlegend.Draw()

        tlatex.DrawLatex(0.6, 0.185, "%s [%.0fK bank]" % (parse_ss(options.ss), options.npatterns*1e-3))
        CMS_label()
        save(options.outdir, "%s_%s" % (hnlambda(hname), options.ss), dot_root=True)

    donotdelete.append(tboxes)
    return


def drawer_sitrep(histos, options):
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

    gStyle.SetNdivisions(510, "Y")
    gStyle.SetEndErrorSize(0)
    use_TEfficiency()

    # Process
    histos = drawer_book(options)
    drawer_project(tchain, histos, options)
    drawer_draw(histos, options, lambda x: "coverage_"+x)
    drawer_draw(histos, options, lambda x: "efficiency_"+x)
    drawer_sitrep(histos, options)


# ______________________________________________________________________________
if __name__ == '__main__':

    # Setup argument parser
    parser = argparse.ArgumentParser()

    # Add default arguments
    add_drawer_arguments(parser)

    # Add more arguments
    parser.add_argument("ss", help="short name of superstrip definition (e.g. ss256)")
    parser.add_argument("npatterns", type=int, help="number of patterns to reach the desired coverage")
    parser.add_argument("--coverage", type=float, default=0.95, help="desired coverage (default: %(default)s)")
    parser.add_argument("--minPt", type=float, default=2, help="min pT (default: %(default)s)")

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)
    options.pu = 0  # assume zero pileup
    options.ptmin = options.minPt

    # Call the main function
    main(options)
