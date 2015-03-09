#ifndef AMSimulationIO_Helper_h_
#define AMSimulationIO_Helper_h_

#include <algorithm>
//#include <functional>
#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "TROOT.h"  // for gROOT
#include "TString.h"
#include "TTree.h"


namespace slhcl1tt {

TString Color(const TString& c);
TString EndColor();
void NoColor();

void Timing(bool start=false);
void ShowTiming();

TString Error();
TString Warning();
TString Info();
TString Debug();

void ResetDeleteBranches(TTree* tree);

}  // namespace slhcl1tt

#endif

