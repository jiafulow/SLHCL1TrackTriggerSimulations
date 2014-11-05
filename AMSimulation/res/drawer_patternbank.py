#!/usr/bin/env python

from rootdrawing import *
from roothelper import *


# ______________________________________________________________________________
# Configurations

sections = {}
sections["fixed"     ] = True
sections["projective"] = False

drawerInit = DrawerInit()
gStyle.SetPadRightMargin(0.1)

imgdir = "figures_patternBank/"

if not imgdir.endswith("/"):  imgdir += "/"
if gSystem.AccessPathName(imgdir):
    gSystem.mkdir(imgdir)


# ______________________________________________________________________________
# Fixed

if sections["fixed"]:
    #TODO: implement this
    pass


# ______________________________________________________________________________
# Projective

if sections["projective"]:
    #TODO: implement this
    pass
