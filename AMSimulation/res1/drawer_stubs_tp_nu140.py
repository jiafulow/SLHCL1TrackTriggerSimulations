#!/usr/bin/env python

from rootdrawing import *
from parser import *

palette = [1,2,418,4,800,891]

# ______________________________________________________________________________
# Load trigger tower map
ttmap = json.load(open("../data/trigger_sector_map.json"), object_pairs_hook=convert_key_to_int)
ttrmap = get_reverse_map(ttmap)


# ______________________________________________________________________________
def drawer_book():
    histos = {}

    for v in ["nclusters", "nstubs"]:
        for i in xrange(5,11):
            hname = "%s_b%i" % (v, i)
            histos[hname] = TH2F(hname, "; Barrel module index; Barrel ladder index", b_nmods[i-5], 0, b_nmods[i-5], b_nlads[i-5], 0, b_nlads[i-5])
            histos[hname].col = palette[i-5]

        for i in xrange(11,23): # endcap
            if i == 16 or i == 17:  continue
            hname = "%s_e%i" % (v, i)
            histos[hname] = TH2F(hname, "; r: Endcap ring index+%i; #theta: Endcap module index" % e_offset, 2*e_nrings[0], -e_nrings[0], e_nrings[0], 2*e_nrings[0], -e_nrings[0], e_nrings[0])  # a square frame
            histos[hname].col = palette[i-11] if i < 16 else palette[i-18]

            for j in xrange(0,15):  # endcap rings
                hname = "%s_e%i_r%i" % (v, i, j)
                histos[hname] = TH2F(hname, "", e_nmods[j], 0, 2*pi, e_nrings[0], 0, e_nrings[0])
                histos[hname].col = palette[i-11] if i < 16 else palette[i-18]

        hname = "%s_tt" % (v)
        histos[hname] = TH2F(hname, "; Trigger tower eta index; Trigger tower phi index", tt_netas, 1, tt_netas+1, tt_nphis, 1, tt_nphis+1)
        histos[hname].col = palette[0]

    # Style
    for hname, h in histos.iteritems():
        h.SetLineWidth(2); h.SetMarkerSize(1.3)
        h.SetLineColor(h.col); h.SetMarkerColor(h.col)
    donotdelete.append(histos)
    return histos


def drawer_project(tree, histos, options):
    tree.SetBranchStatus("*", 0)
    tree.SetBranchStatus("TTClusters_modId", 1)
    tree.SetBranchStatus("TTStubs_modId"   , 1)

    # Loop over events
    for ievt, evt in enumerate(tree):
        if (ievt == options.nentries):  break

        # Loop over clusters
        for moduleId in evt.TTClusters_modId:
            lay = decodeLayer(moduleId)
            lad = decodeLadder(moduleId)
            mod = decodeModule(moduleId)

            if lay < 11:  # barrel
                histos["nclusters_b%i" % (lay)].Fill(mod, lad, 1)
            else:         # endcap
                rad = 2*pi * (mod+0.5) / e_nmods[lad]
                histos["nclusters_e%i_r%i" % (lay,lad)].Fill(rad, lad + e_offset, 1)

            if moduleId in ttrmap:
                for tt in ttrmap[moduleId]:
                    tteta = (tt / tt_nphis) + 1
                    ttphi = (tt % tt_nphis) + 1
                    histos["nclusters_tt"].Fill(tteta, ttphi, 1)

        # Loop over stubs
        for moduleId in evt.TTStubs_modId:
            lay = decodeLayer(moduleId)
            lad = decodeLadder(moduleId)
            mod = decodeModule(moduleId)

            if lay < 11:  # barrel
                histos["nstubs_b%i" % (lay)].Fill(mod, lad, 1)
            else:         # endcap
                rad = 2*pi * (mod+0.5) / e_nmods[lad]
                histos["nstubs_e%i_r%i" % (lay,lad)].Fill(rad, lad + e_offset, 1)

            if moduleId in ttrmap:
                for tt in ttrmap[moduleId]:
                    tteta = (tt / tt_nphis) + 1
                    ttphi = (tt % tt_nphis) + 1
                    histos["nstubs_tt"].Fill(tteta, ttphi, 1)

    # Divide by number of entries to get average
    for k, v in histos.iteritems():
        v.Scale(1.0/float(ievt))

    tree.SetBranchStatus("*", 1)
    return


def modify_barrel(h1, ytitle):
    xlow, xup = -100, 100
    nbinsx = h1.GetNbinsX()
    title = "; Barrel module z position (a.u.); %s" % ytitle
    h = TH1F(h1.GetName()+"_i", title, nbinsx, xlow, xup)
    for ix in xrange(nbinsx):
        h.SetBinContent(ix+1, h1.GetBinContent(ix+1))
    return h

def modify_endcap(h1, ytitle):
    xlow, xup = 0, 15
    nbinsx = h1.GetNbinsX()-e_offset
    title = "; Endcap module #rho position (a.u.); %s" % ytitle
    h = TH1F(h1.GetName()+"_i", title, nbinsx, xlow, xup)
    for ix in xrange(nbinsx):
        h.SetBinContent(ix+1, h1.GetBinContent(ix+1+e_offset))
    return h

def modify_tower(h1, ytitle):
    xlow, xup = 0.5, 6.5
    nbinsx = h1.GetNbinsX()
    title = "; Trigger tower #eta position (a.u.); %s" % ytitle
    h = TH1F(h1.GetName()+"_i", title, nbinsx, xlow, xup)
    for ix in xrange(nbinsx):
        h.SetBinContent(ix+1, h1.GetBinContent(ix+1))
    return h

def drawer_draw(histos, options):

    # Draw barrel
    ymax = 10.0/2 * 1.5
    for i in xrange(5,11):
        hname = "nstubs_b%i" % i
        hname_px = "nstubs_b%i_px" % i
        h2 = histos[hname]
        h1 = h2.ProjectionX(hname_px)
        h1.Scale(1.0/h2.GetNbinsY())  # averaged
        h = histos[hname_px] = modify_barrel(h1, "<# stubs/module/BX>")

        if i-5 == 0:
            h.SetMinimum(0); h.SetMaximum(ymax)
            h.SetStats(0); h.Draw("p")
        h.SetLineWidth(h2.GetLineWidth()); h.SetMarkerSize(h2.GetMarkerSize())
        h.SetLineColor(h2.GetLineColor()); h.SetMarkerColor(h2.GetMarkerColor())
        h.Draw("same p")

    writeme = [
        "#rho =  23 cm",
        "#rho =  36 cm",
        "#rho =  51 cm",
        "#rho =  69 cm",
        "#rho =  89 cm",
        "#rho = 108 cm",
    ]
    moveLegend(0.60,0.66,0.94,0.90); tlegend.Clear()
    for i in xrange(5,11):
        tlegend.AddEntry(histos["nstubs_b%i_px" % i] , writeme[i-5], "p")
    tlegend.Draw()
    CMS_label()
    save(options.outdir, hname, dot_root=True)


    # Draw endcaps
    ymax = 10.0 * 1.5
    for i in xrange(11,23):
        if i == 16 or i == 17:  continue
        hname = "nstubs_e%i_r%i" % (i,0)
        hname_py = "nstubs_e%i_r%i_py" % (i,0)
        h2 = histos[hname]
        h1 = h2.ProjectionY(hname_py)
        h1.Scale(1.0/h2.GetNbinsX())  # averaged

        for j in xrange(0,15):
            if j == 0:  continue
            hname = "nstubs_e%i_r%i" % (i,j)
            hname_py = "nstubs_e%i_r%i_py" % (i,j)
            hh2 = histos[hname]
            hh1 = hh2.ProjectionY(hname_py)
            hh1.Scale(1.0/hh2.GetNbinsX())  # averaged
            h1.Add(hh1)

        hname = "nstubs_e%i" % i
        hname_py = "nstubs_e%i_py" % i
        h = histos[hname_py] = modify_endcap(h1, "<# stubs/module/BX>")

        if i-11 == 0 or i-18 == 0:
            h.SetMinimum(0); h.SetMaximum(ymax)
            h.SetStats(0); h.Draw("p")
        h.SetLineWidth(h2.GetLineWidth()); h.SetMarkerSize(h2.GetMarkerSize())
        h.SetLineColor(h2.GetLineColor()); h.SetMarkerColor(h2.GetMarkerColor())
        h.Draw("same p")

        if i-11 == 4:
            writeme = [
                "z =  135 cm",
                "z =  160 cm",
                "z =  189 cm",
                "z =  224 cm",
                "z =  265 cm",
            ]
            moveLegend(0.60,0.70,0.94,0.90); tlegend.Clear()
            for j in xrange(11,16):
                tlegend.AddEntry(histos["nstubs_e%i_py" % j] , writeme[j-11], "p")
            tlegend.Draw()
            CMS_label()
            save(options.outdir, hname, dot_root=True)

        elif i-18 == 4:
            writeme = [
                "z = -135 cm",
                "z = -160 cm",
                "z = -189 cm",
                "z = -224 cm",
                "z = -265 cm",
            ]
            moveLegend(0.60,0.70,0.94,0.90); tlegend.Clear()
            for j in xrange(18,23):
                tlegend.AddEntry(histos["nstubs_e%i_py" % j] , writeme[j-18], "p")
            tlegend.Draw()
            CMS_label()
            save(options.outdir, hname, dot_root=True)


    # Draw trigger tower
    ymax = 400.0 * 1.5
    hname = "nstubs_tt"
    hname_px = "nstubs_tt_px"
    h2 = histos[hname]
    h1 = h2.ProjectionX(hname_px)
    h1.Scale(1.0/h2.GetNbinsY())  # averaged
    h = histos[hname_px] = modify_tower(h1, "<# stubs/tower/BX>")

    h.SetMinimum(0); h.SetMaximum(ymax)
    h.SetStats(0); h.Draw("p")
    CMS_label()
    save(options.outdir, hname, dot_root=True)


def drawer_draw2(histos, options):

    # Draw barrel
    ymax = 40.0/2
    for i in xrange(5,11):
        hname = "nclusters_b%i" % i
        hname_px = "nclusters_b%i_px" % i
        h2 = histos[hname]
        h1 = h2.ProjectionX(hname_px)
        h2s = histos[hname.replace("nclusters", "nstubs")]
        h1s = h2s.ProjectionX(hname_px.replace("nclusters", "nstubs"))
        h1.Divide(h1s)
        h = histos[hname_px] = modify_barrel(h1, "Cluster over stub multiplicities")

        if i-5 == 0:
            h.SetMinimum(0); h.SetMaximum(ymax)
            h.SetStats(0); h.Draw("p")
        h.SetLineWidth(h2.GetLineWidth()); h.SetMarkerSize(h2.GetMarkerSize())
        h.SetLineColor(h2.GetLineColor()); h.SetMarkerColor(h2.GetMarkerColor())
        h.Draw("same p")

    writeme = [
        "#rho =  23 cm",
        "#rho =  36 cm",
        "#rho =  51 cm",
        "#rho =  69 cm",
        "#rho =  89 cm",
        "#rho = 108 cm",
    ]
    moveLegend(0.60,0.16,0.94,0.40); tlegend.Clear()
    for i in xrange(5,11):
        tlegend.AddEntry(histos["nclusters_b%i_px" % i] , writeme[i-5], "p")
    tlegend.Draw()
    CMS_label()
    save(options.outdir, hname, dot_root=True)


    # Draw endcaps
    ymax = 40.0
    for i in xrange(11,23):
        if i == 16 or i == 17:  continue
        hname = "nclusters_e%i_r%i" % (i,0)
        hname_py = "nclusters_e%i_r%i_py" % (i,0)
        h2 = histos[hname]
        h1 = h2.ProjectionY(hname_py)
        h2s = histos[hname.replace("nclusters", "nstubs")]
        h1s = h2s.ProjectionY(hname_py.replace("nclusters", "nstubs"))
        h1.Divide(h1s)

        for j in xrange(0,15):
            if j == 0:  continue
            hname = "nclusters_e%i_r%i" % (i,j)
            hname_py = "nclusters_e%i_r%i_py" % (i,j)
            hh2 = histos[hname]
            hh1 = hh2.ProjectionY(hname_py)
            hh2s = histos[hname.replace("nclusters", "nstubs")]
            hh1s = hh2s.ProjectionY(hname_py.replace("nclusters", "nstubs"))
            hh1.Divide(hh1s)
            h1.Add(hh1)

        hname = "nclusters_e%i" % i
        hname_py = "nclusters_e%i_py" % i
        h = histos[hname_py] = modify_endcap(h1, "Cluster over stub multiplicities")

        if i-11 == 0 or i-18 == 0:
            h.SetMinimum(0); h.SetMaximum(ymax)
            h.SetStats(0); h.Draw("p")
        h.SetLineWidth(h2.GetLineWidth()); h.SetMarkerSize(h2.GetMarkerSize())
        h.SetLineColor(h2.GetLineColor()); h.SetMarkerColor(h2.GetMarkerColor())
        h.Draw("same p")

        if i-11 == 4:
            writeme = [
                "z =  135 cm",
                "z =  160 cm",
                "z =  189 cm",
                "z =  224 cm",
                "z =  265 cm",
            ]
            moveLegend(0.60,0.70,0.94,0.90); tlegend.Clear()
            for j in xrange(11,16):
                tlegend.AddEntry(histos["nclusters_e%i_py" % j] , writeme[j-11], "p")
            tlegend.Draw()
            CMS_label()
            save(options.outdir, hname, dot_root=True)

        elif i-18 == 4:
            writeme = [
                "z = -135 cm",
                "z = -160 cm",
                "z = -189 cm",
                "z = -224 cm",
                "z = -265 cm",
            ]
            moveLegend(0.60,0.70,0.94,0.90); tlegend.Clear()
            for j in xrange(18,23):
                tlegend.AddEntry(histos["nclusters_e%i_py" % j] , writeme[j-18], "p")
            tlegend.Draw()
            CMS_label()
            save(options.outdir, hname, dot_root=True)


    # Draw trigger tower
    ymax = 40.0
    hname = "nclusters_tt"
    hname_px = "nclusters_tt_px"
    h2 = histos[hname]
    h1 = h2.ProjectionX(hname_px)
    h2s = histos[hname.replace("nclusters", "nstubs")]
    h1s = h2s.ProjectionX(hname_px.replace("nclusters", "nstubs"))
    h1.Divide(h1s)
    h = histos[hname_px] = modify_tower(h1, "Cluster over stub multiplicities")

    h.SetMinimum(0); h.SetMaximum(ymax)
    h.SetStats(0); h.Draw("p")
    CMS_label()
    save(options.outdir, hname, dot_root=True)


def drawer_sitrep(histos, options):
    print "--- SITREP ---------------------------------------------------------"


# ______________________________________________________________________________
# Main function
def main(options):

    # Init
    drawerInit = DrawerInit()
    tchain = TChain("ntupler/tree", "")
    tchain.AddFileInfoList(options.tfilecoll.GetList())

    gStyle.SetLabelSize(0.05, "XY")
    gStyle.SetNdivisions(510, "XY")

    # Process
    histos = drawer_book()
    drawer_project(tchain, histos, options)
    drawer_draw(histos, options)
    drawer_draw2(histos, options)
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

    # Parse default arguments
    options = parser.parse_args()
    parse_drawer_options(options)

    # Call the main function
    main(options)
