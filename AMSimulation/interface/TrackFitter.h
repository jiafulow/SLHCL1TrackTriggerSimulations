#ifndef AMSimulation_TrackFitter_h_
#define AMSimulation_TrackFitter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoLinearized.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoDas.h"
using namespace slhcl1tt;


// SETTINGS: ...
// INPUT   : Roads
// OUTPUT  : Tracks

class TrackFitter {
  public:
    // Constructor
    TrackFitter(TrackFitterOption option)
    : po(option),
      prefixRoad_("AMTTRoads_"), prefixTrack_("AMTTTracks_"), suffix_(""),
      nEvents_(999999999), maxTracks_(999999999),
      verbose_(1) {

        // Decide the track fitter to use
        fitterLin_ = 0;
        fitterDas_ = 0;
        if (po.mode == 0)
            fitterLin_ = new TrackFitterAlgoLinearized();
        else
            fitterDas_ = new TrackFitterAlgoDas();
    }

    // Destructor
    ~TrackFitter() {
        if (fitterLin_) delete fitterLin_;
        if (fitterDas_) delete fitterDas_;
    }


    // Setters
    void setNEvents(long long n)  { if (n != -1)  nEvents_   = n > 0 ? n : 0; }
    void setMaxTracks(int n)      { if (n != -1)  maxTracks_ = n > 0 ? n : 0; }
    void setVerbosity(int v)      { verbose_ = v; }

    // Getters
    // none

    // Functions
    int makeTracks(TString src, TString out);

    // Main driver
    int run(TString src, TString out);


  private:
    // Configurations
    const TrackFitterOption po;
    const TString prefixRoad_;
    const TString prefixTrack_;
    const TString suffix_;

    // Program options
    long long nEvents_;
    int maxTracks_;  // max number of tracks per event
    int verbose_;

    // Track Fitters
    TrackFitterAlgoLinearized * fitterLin_;
    TrackFitterAlgoDas * fitterDas_;
};

#endif
