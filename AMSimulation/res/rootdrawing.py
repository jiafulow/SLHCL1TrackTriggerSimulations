from ROOT import TH1, TH1F, TH2F, TProfile, TProfile2D, TFile, TChain, TCanvas, TLegend, TLatex, TLine, TBox, TGraph, TGraphAsymmErrors, gROOT, gInterpreter, gStyle, gSystem, gPad
from rootcolors import *
from math import sqrt
from random import randint
from itertools import izip

# For init
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
        gStyle.SetNdivisions(505, "XY")

        gStyle.SetOptStat(111110)
        gStyle.SetStatX(0.94)
        gStyle.SetStatY(0.93)
        gStyle.SetStatH(0.20)
        gStyle.SetStatW(0.24)

        TH1.SetDefaultSumw2()

# ______________________________________________________________________________
# Globals
latex = TLatex()
latex.SetNDC()
latex.SetTextFont(42)
latex.SetTextSize(0.026)

legend = TLegend(0.70,0.74,0.96,0.94)
legend.SetFillStyle(0)
legend.SetLineColor(0)
legend.SetShadowColor(0)
legend.SetBorderSize(0)

line = TLine()
line.SetLineColor(kGray+2)
line.SetLineStyle(2)

# ______________________________________________________________________________
# Classes

class Param(object):
    # as suggested in http://stackoverflow.com/questions/35988/c-like-structures-in-python
    def __init__(self, **kwds):
        self.__dict__.update(kwds)

class _HistViewBase(object):
    def __init__(self, p):
        self.h = None
        self.name = p.name
        self.var = p.var
        self.cut = p.cut

    def style(self, p):
        self.h.SetLineWidth(2)
        self.h.SetMarkerSize(0)
        if hasattr(p, "linecolor"  ):  self.h.SetLineColor(p.linecolor)
        if hasattr(p, "markercolor"):  self.h.SetMarkerColor(p.markercolor)
        if hasattr(p, "fillcolor"  ):  self.h.SetFillColor(p.fillcolor)
        if hasattr(p, "linestyle"  ):  self.h.SetLineStyle(p.linestyle)
        if hasattr(p, "markerstyle"):  self.h.SetMarkerStyle(p.markerstyle)
        if hasattr(p, "fillstyle"  ):  self.h.SetFillStyle(p.fillstyle)
        if hasattr(p, "linewidth"  ):  self.h.SetLineWidth(p.linewidth)
        if hasattr(p, "markersize" ):  self.h.SetMarkerSize(p.markersize)

    def Draw(self, arg=""):
        self.h.Draw(arg)

class HistView(_HistViewBase):
    def __init__(self, p):
        super(HistView, self).__init__(p)
        self.h = TH1F("h1_%i" % randint(0,65535), "; %s" % p.xtitle, p.nbinsx, p.xlow, p.xup)
        self.randname = self.h.GetName()
        self.style(p)

    def fixOverflow(self):
        nbins = self.h.GetNbinsX()
        if self.h.GetBinContent(nbins+1) > 0:
            self.h.SetBinContent(nbins, self.h.GetBinContent(nbins) + self.h.GetBinContent(nbins+1))
            self.h.SetBinError(nbins, sqrt(self.h.GetBinError(nbins)**2 + self.h.GetBinError(nbins+1)**2))
            self.h.SetBinContent(nbins+1, 0)
            self.h.SetBinError(nbins+1, 0)
            self.h.SetEntries(self.h.GetEntries() - 2)  # SetBinContent() automatically increases # entries by one

    def fixUnderflow(self):
        if self.h.GetBinContent(0) > 0:
            self.h.SetBinContent(1, self.h.GetBinContent(0) + self.h.GetBinContent(1))
            self.h.SetBinError(1, sqrt(self.h.GetBinError(0)**2 + self.h.GetBinError(1)**2))
            self.h.SetBinContent(0, 0)
            self.h.SetBinError(0, 0)
            self.h.SetEntries(self.h.GetEntries() - 2)  # SetBinContent() automatically increases # entries by one

class HistViewProf(_HistViewBase):
    def __init__(self, p):
        super(HistViewProf, self).__init__(p)
        self.h = TProfile("hp1_%i" % randint(0,65535), "; %s; %s" % (p.xtitle, p.ytitle), p.nbinsx, p.xlow, p.xup, p.ylow, p.yup)
        self.randname = self.h.GetName()
        p.markersize = 1.3
        self.style(p)

class HistView2D(_HistViewBase):
    def __init__(self, p):
        super(HistView2D, self).__init__(p)
        self.h = TH2F("h2_%i" % randint(0,65535), "; %s; %s" % (p.xtitle, p.ytitle), p.nbinsx, p.xlow, p.xup, p.nbinsy, p.ylow, p.yup)
        self.randname = self.h.GetName()
        self.style(p)
        self.h.SetStats(0)

# ______________________________________________________________________________
# Functions

# Struct
def struct(*args):
    p = Param(name=args[0],
        var=args[1],
        cut=args[2],
        xtitle=args[3],
        nbinsx=args[4],
        xlow=args[5],
        xup=args[6],
        linecolor=args[7], markercolor=args[7],
        fillcolor=args[8]
        )
    return p

def struct2D(*args):
    p = Param(name=args[0],
        var=args[1],
        cut=args[2],
        xtitle=args[3],
        nbinsx=args[4],
        xlow=args[5],
        xup=args[6],
        ytitle=args[7],
        nbinsy=args[8],
        ylow=args[9],
        yup=args[10],
        linecolor=args[11], markercolor=args[11],
        fillcolor=args[12]
        )
    return p

# Book
def book(params):
    histos = []
    for i, p in enumerate(params):
        hv = HistView(p)
        histos.append(hv)
    return histos

def bookProf(params, option="s"):
    histos = []
    for i, p in enumerate(params):
        hv = HistViewProf(p)
        if "s" in option:
            hv.h.SetErrorOption("s")
        histos.append(hv)
    return histos

def book2D(params):
    histos = []
    for i, p in enumerate(params):
        hv = HistView2D(p)
        histos.append(hv)
    return histos

# Project
def project(tree, histos, nentries=1000000000, normalize=-1, drawOverflow=True, drawUnderflow=True):
    for i, h in enumerate(histos):
        tree.Project(h.randname, h.var, h.cut, "goff", nentries)
        if normalize > 0:  h.Scale(normalize / h.GetSumOfWeights())
        if drawOverflow :  h.fixOverflow()
        if drawUnderflow:  h.fixUnderflow()
    return

def projectProf(tree, histos, nentries=1000000000):
    for i, h in enumerate(histos):
        tree.Project(h.randname, h.var, h.cut, "prof goff", nentries)
    return

def project2D(tree, histos, nentries=1000000000):
    for i, h in enumerate(histos):
        tree.Project(h.randname, h.var, h.cut, "goff", nentries)
    return

# Draw
def draw(histos, ytitle="Entries", logx=False, logy=False, stats=True, text=False):
    if isinstance(histos[0], HistView):
        histos = [hv.h for hv in histos]
    h = histos[0]
    if not stats:  h.SetStats(0)
    if logy:
        h.SetMaximum(h.GetMaximum() * 14)
    else:
        h.SetMaximum(h.GetMaximum() * 1.4); h.SetMinimum(0.)
    if ytitle:  h.GetYaxis().SetTitle(ytitle)
    h.Draw("hist")
    gPad.SetLogx(logx); gPad.SetLogy(logy)
    if text:
        h.SetMarkerSize(2)
        h.Draw("hist text0")
    CMS_label()
    return

def drawProf(histos, ytitle="Entries", logx=False, logy=False, ymax=-1, stats=True):
    if isinstance(histos[0], HistViewProf):
        histos = [hv.h for hv in histos]
    h = histos[0]
    if not stats:  h.SetStats(0)
    if logy:
        h.SetMaximum(h.GetMaximum() * 14)
    else:
        h.SetMaximum(h.GetMaximum() * 1.4); h.SetMinimum(0.)
    if ymax != -1: h.SetMaximum(ymax)
    if ytitle:  h.GetYaxis().SetTitle(ytitle)
    h.Draw("hist p")
    gPad.SetLogx(logx); gPad.SetLogy(logy)
    CMS_label()
    return

# Draw more than one
def draws(histos, ytitle="Entries", logx=False, logy=False, stats=False):
    if isinstance(histos[0], HistView):
        histos = [hv.h for hv in histos]
    h = histos[0]
    if not stats:  h.SetStats(0)
    if logy:
        h.SetMaximum(h.GetMaximum() * 14)
    else:
        h.SetMaximum(h.GetMaximum() * 1.4); h.SetMinimum(0.)
    if ytitle:  h.GetYaxis().SetTitle(ytitle)
    h.Draw("hist")
    gPad.SetLogx(logx); gPad.SetLogy(logy)
    for h in histos[1:]:
        h.Draw("same hist")
    CMS_label()
    return

def drawsProf(histos, ytitle="", logx=False, logy=False, ymax=-1, stats=False):
    if isinstance(histos[0], HistViewProf):
        histos = [hv.h for hv in histos]
    h = histos[0]
    if not stats:  h.SetStats(0)
    if logy:
        h.SetMaximum(h.GetMaximum() * 14)
    else:
        h.SetMaximum(h.GetMaximum() * 1.4); h.SetMinimum(0.)
    if ymax != -1: h.SetMaximum(ymax)
    if ytitle:  h.GetYaxis().SetTitle(ytitle)
    h.Draw("hist p")
    gPad.SetLogx(logx); gPad.SetLogy(logy)
    for h in histos[1:]:
        h.Draw("same hist p")
    CMS_label()
    return

def movePalette2D(h):
    x1, y1, x2, y2 = 0.91, 0.13, 0.95, 0.95
    paletteObj = h.FindObject("palette")
    paletteObj.SetX1NDC(x1); paletteObj.SetY1NDC(y1); paletteObj.SetX2NDC(x2); paletteObj.SetY2NDC(y2)
    paletteObj.SetTitleSize(0.024); paletteObj.SetLabelSize(0.024)

def moveStats2D(h):
    x1, y1, x2, y2 = 0.64, 0.70, 0.88, 0.93
    statsObj = h.FindObject("stats")
    statsObj.SetX1NDC(x1); statsObj.SetY1NDC(y1); statsObj.SetX2NDC(x2); statsObj.SetY2NDC(y2)

def draw2D(histos, logx=False, logy=False, logz=False, palette=True, stats=True):
    if isinstance(histos[0], HistView2D):
        histos = [hv.h for hv in histos]
    h = histos[0]
    if not stats:
        h.SetStats(0)
    if palette:
        h.Draw("colz")
    else:
        h.Draw("col")
    gPad.SetLogx(logx); gPad.SetLogy(logy); gPad.SetLogz(logz)
    gPad.SetRightMargin(0.10)
    gPad.Modified(); gPad.Update()
    if palette:
        movePalette2D(h)
    if stats:
        moveStats2D(h)
    gPad.Modified(); gPad.Update()
    CMS_label()
    return

# ______________________________________________________________________________
# Auxiliary

def moveLegend(x1, y1, x2, y2):
    # SetX1NDC, SetX2NDC etc don't update if the legend is not drawn first
    # SetX1, SetX2 etc don't update if the legend has already been drawn
    # Thanks ROOT!
    legend.SetX1(x1); legend.SetY1(y1); legend.SetX2(x2); legend.SetY2(y2)
    legend.SetX1NDC(x1); legend.SetY1NDC(y1); legend.SetX2NDC(x2); legend.SetY2NDC(y2)

def CMS_label():
    cmsTextFont = 61; extraTextFont = 52
    lumiTextSize = 0.6; lumiTextOffset = 0.2; extraOverCmsTextSize = 0.76
    cmsTextSize = 0.75; cmsTextOffset = 0.1; extraTextSize = extraOverCmsTextSize * cmsTextSize
    relPosX = 0.045; relPosY = 0.035; relExtraDY = 1.2
    lumiText = "";  cmsText = "CMS";  extraText = "Preliminary Phase II Simulation"

    old = (latex.GetTextFont(), latex.GetTextSize())
    l, r, t, b = gPad.GetLeftMargin(), gPad.GetRightMargin(), gPad.GetTopMargin(), gPad.GetBottomMargin()
    posX_ = l + relPosX*(1-l-r); posY_ = 1-t+lumiTextOffset*t
    relPosX = 0.105; posX_ = 0.62 - relPosX*(1-l-r);  # right aligned
    latex.SetTextFont(42); latex.SetTextSize(lumiTextSize*t)
    latex.DrawLatex(1-r,1-t+lumiTextOffset*t, lumiText)
    latex.SetTextFont(cmsTextFont); latex.SetTextSize(cmsTextSize*t)
    latex.DrawLatex(posX_,1-t+lumiTextOffset*t,cmsText)  # out of frame
    latex.SetTextFont(extraTextFont); latex.SetTextSize(extraTextSize*t)
    latex.DrawLatex(posX_ + relPosX*(1-l-r), posY_, extraText)
    latex.SetTextFont(old[0]); latex.SetTextSize(old[1])
    return

def getMaximum(histos):
    maxima = []
    for h in histos:
        maxima.append(h.h.GetMaximum())
    return max(maxima)

def save(imgdir, imgname, dot_root=False):
    gPad.RedrawAxis()
    #gPad.Modified(); gPad.Update()
    gPad.Print(imgdir+imgname+".pdf")
    gPad.Print(imgdir+imgname+".png")
    if dot_root:
        gPad.Print(imgdir+imgname+".root")

# ______________________________________________________________________________
# I/O

def get_infiles(txtfile, fast=False):
    infiles = []
    with open(txtfile) as f:
        for line in f:
            infiles.append(line.strip())
    if fast:
        infiles = infiles[:2]
    return infiles
