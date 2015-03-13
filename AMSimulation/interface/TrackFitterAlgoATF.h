/* --- An Analytical Track Fitter for rapid least-chi2 fitting ---

Algorithm described in CMS Detector Note DN-2015/004
cms.cern.ch/iCMS/jsp/openfile.jsp?tp=draft&files=DN2015_004_v2.pdf

Souvik Das, University of Florida
15 October 2014
--- ---  --- --- --- --- --- --- --- --- --- --- --- --- --- --- */

#ifndef AMSimulation_TrackFitterAlgoATF_h_
#define AMSimulation_TrackFitterAlgoATF_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTHit.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack2.h"

using namespace slhcl1tt;

class TrackFitterAlgoATF
{
  public:
    TrackFitterAlgoATF(bool fiveParameters) : fiveParameters_(fiveParameters){}
    ~TrackFitterAlgoATF() {}

    int fit(const std::vector<TTHit>& hits, TTTrack2& track);
    
  private:
    bool fiveParameters_=false;
};

#endif
