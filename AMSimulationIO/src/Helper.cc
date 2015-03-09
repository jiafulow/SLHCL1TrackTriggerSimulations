#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"

#include "TStopwatch.h"
#include "TBranchElement.h"

namespace slhcl1tt {

static bool g_useColor = true;
static bool g_useTimer = false;

// Stolen from $ROOTSYS/tmva/src/Tools.cxx
TString Color(const TString& c) {
    // human readable color strings
    static TString gClr_none         = "" ;
    static TString gClr_white        = "\033[1;37m";  // white
    static TString gClr_black        = "\033[30m";    // black
    static TString gClr_blue         = "\033[34m";    // blue
    static TString gClr_red          = "\033[1;31m" ; // red
    static TString gClr_yellow       = "\033[1;33m";  // yellow
    static TString gClr_darkred      = "\033[31m";    // dark red
    static TString gClr_darkgreen    = "\033[32m";    // dark green
    static TString gClr_darkyellow   = "\033[33m";    // dark yellow

    static TString gClr_bold         = "\033[1m"    ; // bold
    static TString gClr_black_b      = "\033[30m"   ; // bold black
    static TString gClr_lblue_b      = "\033[1;34m" ; // bold light blue
    static TString gClr_magenta_b    = "\033[1;35m" ; // bold magenta
    static TString gClr_cyan_b       = "\033[0;36m" ; // bold cyan
    static TString gClr_gray_b       = "\033[0;37m" ; // bold gray
    static TString gClr_lgreen_b     = "\033[1;32m";  // bold light green

    static TString gClr_blue_bg      = "\033[44m";    // blue background
    static TString gClr_red_bg       = "\033[1;41m";  // white on red background
    static TString gClr_whiteonblue  = "\033[1;44m";  // white on blue background
    static TString gClr_whiteongreen = "\033[1;42m";  // white on green background
    static TString gClr_grey_bg      = "\033[47m";    // grey background

    static TString gClr_reset        = "\033[0m";     // reset

    if (!g_useColor)           return gClr_none;
    if (c == "none")           return gClr_none;

    if (c == "white" )         return gClr_white;
    if (c == "blue"  )         return gClr_blue;
    if (c == "black"  )        return gClr_black;
    if (c == "magenta")        return gClr_magenta_b;
    if (c == "lightblue")      return gClr_cyan_b;
    if (c == "gray")           return gClr_gray_b;
    if (c == "yellow")         return gClr_yellow;
    if (c == "red"   )         return gClr_red;
    if (c == "dred"  )         return gClr_darkred;
    if (c == "dgreen")         return gClr_darkgreen;
    if (c == "lgreenb")        return gClr_lgreen_b;
    if (c == "dyellow")        return gClr_darkyellow;

    if (c == "bold")           return gClr_bold;
    if (c == "bblack")         return gClr_black_b;

    if (c == "blue_bgd")       return gClr_blue_bg;
    if (c == "red_bgd" )       return gClr_red_bg;

    if (c == "white_on_blue" ) return gClr_whiteonblue;
    if (c == "white_on_green") return gClr_whiteongreen;

    if (c == "reset")          return gClr_reset;
    return gClr_none;
}

TString EndColor() {
    static TString gClr_none         = "" ;
    static TString gClr_reset        = "\033[0m";     // reset
    if (!g_useColor)           return gClr_none;
    return gClr_reset;
}

void NoColor() {
    g_useColor = false;
}

void Timing(bool start) {
    static TStopwatch timer;
    if (g_useTimer) {
        if (start) {
            timer.Start();
        } else {
            timer.Stop();
            std::cout << Info(); timer.Print();
            timer.Continue();
        }
    }
}

void ShowTiming() {
    g_useTimer = true;
}

TString Error() {
    return Color("red") + "ERROR" + Color("reset") + "  : ";
}

TString Warning() {
    return Color("yellow") + "WARNING" + Color("reset") + ": ";
}

TString Info() {
    return Color("lightblue") + "INFO" + Color("reset") + "   : ";
}

TString Debug() {
    return Color("gray") + "DEBUG" + Color("reset") + "  : ";
}

void ResetDeleteBranches (TTree* tree) {
    TObjArray* branches = tree->GetListOfBranches();
    Int_t nb = branches->GetEntriesFast();
    for (Int_t i = 0; i < nb; ++i) {
        TBranch* br = (TBranch*) branches->UncheckedAt(i);
        if (br->InheritsFrom(TBranchElement::Class())) {
            ((TBranchElement*) br)->ResetDeleteObject();
        }
    }
}

}  // namespace slhcl1tt

