import sys
tmpargv = sys.argv[:]  # [:] for a copy, not reference
sys.argv = []

import ROOT
from ROOT import TH1, TH1F, TH2F, TProfile, TProfile2D, TFile, TChain, TCanvas, TPad, TLegend, TLatex, TLine, TPolyLine, TBox, TGraph, TGraphErrors, TGraphAsymmErrors, TEfficiency, gROOT, gInterpreter, gStyle, gSystem, gPad
from rootcolors import *
sys.argv = tmpargv

import re
import os
from math import sqrt, pi
from random import randint
from itertools import izip
from array import array

# ______________________________________________________________________________
# Init
class DrawerInit:
    def __init__(self):
        # ROOT
        gROOT.LoadMacro("tdrstyle.C")
        gROOT.LoadMacro("../interface/HelperMath.h")
        gROOT.LoadMacro("../../AMSimulationIO/src/AMSimulationIOLinkDef.h")
        gROOT.ProcessLine("setTDRStyle()")

        gStyle.SetEndErrorSize(2)
        gStyle.SetPadRightMargin(0.05)
        gStyle.SetTitleOffset(1.1, "Y")
        gStyle.SetLabelSize(0.04, "Y")
        gStyle.SetLabelSize(0.04, "Z")
        gStyle.SetNdivisions(505, "XY")

        gStyle.SetPalette(55)  # rainbow color map
        gStyle.SetNumberContours(100)

        gStyle.SetOptStat(111110)
        gStyle.SetStatX(0.94)
        gStyle.SetStatY(0.93)
        gStyle.SetStatH(0.30)
        gStyle.SetStatW(0.28)

        TH1.SetDefaultSumw2()

# ______________________________________________________________________________
# Globals
tlatex = TLatex()
tlatex.SetNDC()
tlatex.SetTextFont(42)
tlatex.SetTextSize(0.03)

tlegend = TLegend(0.70,0.74,0.96,0.94)
tlegend.SetFillStyle(0)
tlegend.SetLineColor(0)
tlegend.SetShadowColor(0)
tlegend.SetBorderSize(0)

tline = TLine()
tline.SetLineColor(kGray+2)
tline.SetLineStyle(2)

donotdelete = []  # persist in memory


# ______________________________________________________________________________
# Auxiliary

def count_if(seq, condition):
    return sum(1 for item in seq if condition(item))

def modify_binning(nbins, xmin, xmax):
    binsize = (xmax - xmin) / nbins
    return (nbins + 1, xmin - (binsize/2.), xmax + (binsize/2.))

def moveLegend(x1, y1, x2, y2):
    # SetX1NDC, SetX2NDC etc don't update if the tlegend is not drawn first
    # SetX1, SetX2 etc don't update if the tlegend has already been drawn
    # Thanks ROOT!
    tlegend.SetX1(x1); tlegend.SetY1(y1); tlegend.SetX2(x2); tlegend.SetY2(y2)
    tlegend.SetX1NDC(x1); tlegend.SetY1NDC(y1); tlegend.SetX2NDC(x2); tlegend.SetY2NDC(y2)

def movePalette(h, x1=0.88, y1=0.13, x2=0.92, y2=0.95):
    paletteObj = h.FindObject("palette")
    paletteObj.SetX1NDC(x1); paletteObj.SetY1NDC(y1); paletteObj.SetX2NDC(x2); paletteObj.SetY2NDC(y2)

def moveStats(h, x1=0.64, y1=0.70, x2=0.88, y2=0.93):
    statsObj = h.FindObject("stats")
    statsObj.SetX1NDC(x1); statsObj.SetY1NDC(y1); statsObj.SetX2NDC(x2); statsObj.SetY2NDC(y2)

def CMS_label():
    cmsTextFont = 61; extraTextFont = 52
    lumiTextSize = 0.6; lumiTextOffset = 0.2; extraOverCmsTextSize = 0.76
    cmsTextSize = 0.75; cmsTextOffset = 0.1; extraTextSize = extraOverCmsTextSize * cmsTextSize
    relPosX = 0.045; relPosY = 0.035; relExtraDY = 1.2
    lumiText = "";  cmsText = "CMS";  extraText = "Preliminary Phase II Simulation"

    old = (tlatex.GetTextFont(), tlatex.GetTextSize())
    l, r, t, b = gPad.GetLeftMargin(), gPad.GetRightMargin(), gPad.GetTopMargin(), gPad.GetBottomMargin()
    posX_ = l + relPosX*(1-l-r); posY_ = 1-t+lumiTextOffset*t
    relPosX = 0.105; posX_ = 0.62 - relPosX*(1-l-r);  # right aligned
    tlatex.SetTextFont(42); tlatex.SetTextSize(lumiTextSize*t)
    tlatex.DrawLatex(1-r,1-t+lumiTextOffset*t, lumiText)
    tlatex.SetTextFont(cmsTextFont); tlatex.SetTextSize(cmsTextSize*t)
    tlatex.DrawLatex(posX_,1-t+lumiTextOffset*t,cmsText)  # out of frame
    tlatex.SetTextFont(extraTextFont); tlatex.SetTextSize(extraTextSize*t)
    tlatex.DrawLatex(posX_ + relPosX*(1-l-r), posY_, extraText)
    tlatex.SetTextFont(old[0]); tlatex.SetTextSize(old[1])
    return

def getMaximum(histos):
    maxima = []
    for h in histos:
        maxima.append(h.h.GetMaximum())
    return max(maxima)

def save(imgdir, imgname, redraw_axis=True, dot_pdf=True, dot_root=False, dot_c=False, additional=[]):
    if redraw_axis:
        gPad.RedrawAxis()
    #gPad.Modified(); gPad.Update()
    gPad.Print(imgdir+imgname+".png")
    if dot_pdf:
        gPad.Print(imgdir+imgname+".pdf")
    if dot_root:
        def save_for_root(p):
            if p.ClassName() == "TPad" or p.ClassName() == "TCanvas":
                for pp in p.GetListOfPrimitives():
                    save_for_root(pp)
            p.Write()

        tfile = TFile.Open(imgdir+imgname+".root", "RECREATE")
        save_for_root(gPad)

        if additional:
            for a in additional:
                a.Write()
        tfile.Close()
    if dot_c:
        gPad.Print(imgdir+imgname+".C")

