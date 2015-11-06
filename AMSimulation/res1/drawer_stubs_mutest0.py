#!/usr/bin/env python

from rootdrawing import *
from parser import *

palette = [1,2,418,4,800,891]

# Load trigger tower map
ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)


# ______________________________________________________________________________
def use_TEfficiency():
    del ROOT.TEfficiency
    gROOT.ProcessLine(".L ./fixes/TEfficiency.cxx+")
    #gSystem.Load("./fixes/TEfficiency_cxx.so")

    global TEfficiency
    TEfficiency = ROOT.TEfficiency


# ______________________________________________________________________________
def get_ds_cuts(ds):
    if ds == 0:
        # Default
        return [0, 2.5, 2.5, 3.0, 4.5, 5.5, 6.5]
    if ds == 1:
        # Default
        return [0, 2.5, 2.5, 3.0, 4.5, 5.5, 6.5]
    if ds == 2:
        # Taken from https://github.com/sviret/HL_LHC/blob/master/DataProduction/test/base/2GevTune.txt
        return [0, 2.0, 2.0, 2.5, 4.0, 5.5, 6.5]
    if ds == 3:
        # Taken from https://github.com/sviret/HL_LHC/blob/master/DataProduction/test/base/3GevTune.txt
        return [0, 1.5, 1.5, 2.0, 3.0, 3.5, 4.5]


# ______________________________________________________________________________
def drawer_book():
    histos = {}

    prefix = "efficiency_"
    ytitle = "Efficiency"
    nbinsx, xlow, xup = 50, 0., 10.

    for i in xrange(6):
        hname = prefix + "layer_%i" % i
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s {%s}" % (ytitle, "L%i" % i), nbinsx, xlow, xup)
        histos[hname].col = palette[i]

        hname = prefix + "layer_tt_%i" % i
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s {%s}" % (ytitle, "L%i in tt27" % i), nbinsx, xlow, xup)
        histos[hname].col = palette[i]

        hname = prefix + "nlayers_%i" % i
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s {%s}" % (ytitle, "#geq%i layers" % (i+1)), nbinsx, xlow, xup)
        histos[hname].col = palette[i]

        hname = prefix + "nlayers_tt_%i" % i
        histos[hname] = TEfficiency(hname, "; p_{T} [GeV]; %s {%s}" % (ytitle, "#geq%i layers in tt27" % (i+1)), nbinsx, xlow, xup)
        histos[hname].col = palette[i]

    # Error statistics
    for hname, h in histos.iteritems():
        if h.ClassName() == "TEfficiency":
            h.SetConfidenceLevel(0.682689492137)  # 1-sigma
            h.SetStatisticOption(0)  # kFCP
            #h.SetStatisticOption(6)  # kBUniform

    # Style
    for hname, h in histos.iteritems():
        h.SetLineWidth(2); h.SetMarkerSize(0)
        h.SetMarkerStyle(20); h.SetMarkerSize(1)
        h.SetLineColor(h.col); h.SetMarkerColor(h.col)
    donotdelete.append(histos)
    return histos


def drawer_project(tree, histos, options):
    tree.SetBranchStatus("*", 0)
    tree.SetBranchStatus("genParts_pt"     , 1)
    tree.SetBranchStatus("genParts_eta"    , 1)
    tree.SetBranchStatus("genParts_phi"    , 1)
    tree.SetBranchStatus("TTStubs_modId"   , 1)
    tree.SetBranchStatus("TTStubs_trigBend", 1)

    moduleIds_tt = set()
    for moduleId in ttmap[options.tower]:
        moduleIds_tt.add(moduleId)

    dscuts = get_ds_cuts(options.ds)
    dscuts = dscuts[1:]

    # Loop over events
    for ievt, evt in enumerate(tree):
        if (ievt == options.nentries):  break
        if (ievt % 10000 == 0):  print "Processing event: %i" % ievt

        pt  = evt.genParts_pt [0]
        eta = evt.genParts_eta[0]
        phi = evt.genParts_phi[0]

        if not (options.etamin < eta < options.etamax and options.phimin < phi < options.phimax):
            continue

        counts_layer = {}
        counts_layer_tt = {}

        for moduleId, ds in izip(evt.TTStubs_modId, evt.TTStubs_trigBend):
            lay = decodeLayer(moduleId)

            if lay-5 < len(dscuts) and not (ds <= dscuts[lay-5]):
                continue

            counts_layer[lay-5] = counts_layer.get(lay-5, 0) + 1

            if moduleId not in moduleIds_tt:
                continue

            counts_layer_tt[lay-5] = counts_layer_tt.get(lay-5, 0) + 1


        prefix = "efficiency_"
        for i in xrange(6):
            trigger = i in counts_layer
            histos[prefix + "layer_%i" % i].Fill(trigger, pt)

            trigger_tt = i in counts_layer_tt
            histos[prefix + "layer_tt_%i" % i].Fill(trigger_tt, pt)

            trigger_nlayers = len(counts_layer) > i
            histos[prefix + "nlayers_%i" % i].Fill(trigger_nlayers, pt)

            trigger_nlayers_tt = len(counts_layer_tt) > i
            histos[prefix + "nlayers_tt_%i" % i].Fill(trigger_nlayers_tt, pt)

    tree.SetBranchStatus("*", 1)
    return


def drawer_draw(histos, options):

    #for hname, h in histos.iteritems():
    #
    #    if h.ClassName() == "TEfficiency":
    #        ymax = 1.2
    #
    #        h1 = h.GetCopyTotalHisto(); h1.Reset()
    #        h1.SetMinimum(0); h1.SetMaximum(ymax)
    #        h1.SetStats(0); h1.Draw()
    #
    #        xmin, xmax = h1.GetXaxis().GetXmin(), h1.GetXaxis().GetXmax()
    #        tline.DrawLine(xmin, 1.0, xmax, 1.0)
    #
    #        h.gr = h.CreateGraph()
    #        h.gr.Draw("p")
    #
    #    CMS_label()
    #    save(options.outdir, hname)


    # Specialized
    prefix = "efficiency_"
    ymax = 1.2

    ytitle = "Stub finding efficiency"
    for i in xrange(6):
        h = histos[prefix + "layer_%i" % i]
        if i == 0:
            h1 = h.GetCopyTotalHisto(); h1.Reset()
            h1.SetYTitle(ytitle)
            h1.SetMinimum(0); h1.SetMaximum(ymax)
            h1.SetStats(0); h1.Draw()

            xmin, xmax = h1.GetXaxis().GetXmin(), h1.GetXaxis().GetXmax()
            tline.DrawLine(xmin, 1.0, xmax, 1.0)

        h.gr = h.CreateGraph()
        h.gr.Draw("p")

    writeme = [
        "#rho =  23 cm",
        "#rho =  36 cm",
        "#rho =  51 cm",
        "#rho =  69 cm",
        "#rho =  89 cm",
        "#rho = 108 cm",
    ]
    moveLegend(0.60,0.16,0.94,0.40); tlegend.Clear()
    for i in xrange(6):
        tlegend.AddEntry(histos[prefix + "layer_%i" % i] , writeme[i], "p")
    tlegend.Draw()
    CMS_label()
    save(options.outdir, prefix + "layer_all_ds%s" % options.ds, dot_root=True)

    ytitle = "Stub finding efficiency {tt27}"
    for i in xrange(6):
        h = histos[prefix + "layer_tt_%i" % i]
        if i == 0:
            h1 = h.GetCopyTotalHisto(); h1.Reset()
            h1.SetYTitle(ytitle)
            h1.SetMinimum(0); h1.SetMaximum(ymax)
            h1.SetStats(0); h1.Draw()

            xmin, xmax = h1.GetXaxis().GetXmin(), h1.GetXaxis().GetXmax()
            tline.DrawLine(xmin, 1.0, xmax, 1.0)

        h.gr = h.CreateGraph()
        h.gr.Draw("p")

    tlegend.Draw()
    CMS_label()
    save(options.outdir, prefix + "layer_tt_all_ds%s" % options.ds, dot_root=True)

    ytitle = "n-stubs finding efficiency"
    for i in xrange(6):
        h = histos[prefix + "nlayers_%i" % i]
        if i == 0:
            h1 = h.GetCopyTotalHisto(); h1.Reset()
            h1.SetYTitle(ytitle)
            h1.SetMinimum(0); h1.SetMaximum(ymax)
            h1.SetStats(0); h1.Draw()

            xmin, xmax = h1.GetXaxis().GetXmin(), h1.GetXaxis().GetXmax()
            tline.DrawLine(xmin, 1.0, xmax, 1.0)

        if i >= 3:
            h.gr = h.CreateGraph()
            h.gr.Draw("p")

    writeme = [
        "#geq 1 layer",
        "#geq 2 layers",
        "#geq 3 layers",
        "#geq 4 layers",
        "#geq 5 layers",
        "= 6 layers",
    ]
    moveLegend(0.60,0.16,0.94,0.28); tlegend.Clear()
    for i in xrange(6):
        if i >= 3:
            tlegend.AddEntry(histos[prefix + "nlayers_%i" % i] , writeme[i], "p")
    tlegend.Draw()
    CMS_label()
    save(options.outdir, prefix + "nlayers_all_ds%s" % options.ds, dot_root=True)

    ytitle = "n-stubs finding efficiency {tt27}"
    for i in xrange(6):
        h = histos[prefix + "nlayers_tt_%i" % i]
        if i == 0:
            h1 = h.GetCopyTotalHisto(); h1.Reset()
            h1.SetYTitle(ytitle)
            h1.SetMinimum(0); h1.SetMaximum(ymax)
            h1.SetStats(0); h1.Draw()

            xmin, xmax = h1.GetXaxis().GetXmin(), h1.GetXaxis().GetXmax()
            tline.DrawLine(xmin, 1.0, xmax, 1.0)

        if i >= 3:
            h.gr = h.CreateGraph()
            h.gr.Draw("p")

    tlegend.Draw()
    CMS_label()
    save(options.outdir, prefix + "nlayers_tt_all_ds%s" % options.ds, dot_root=True)

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

    gStyle.SetTitleSize(0.05, "Y")
    gStyle.SetNdivisions(510, "XY")
    gStyle.SetEndErrorSize(0)
    use_TEfficiency()

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
    parser.add_argument("--ds", type=int, default=0, help="delta s cuts (default: %(default)s)")

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)

    # Call the main function
    main(options)
