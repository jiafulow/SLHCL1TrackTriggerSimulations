#ifndef AMSimulation_TrackFitter_h_
#define AMSimulation_TrackFitter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoLinearized.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoATF.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoRetina.h"
using namespace slhcl1tt;


class TrackFitter {

  public:
    // Constructor
    TrackFitter(TrackFitterOption po)
    : po_(po),
      prefixRoad_("AMTTRoads_"), prefixTrack_("AMTTTracks_"), suffix_(""),
      nEvents_(999999999), maxCombs_(999999999), maxTracks_(999999999),
      verbose_(1) {

        // Decide the track fitter to use
        fitterLin_    = 0;
        fitterATF_    = 0;
        fitterRetina_ = 0;
        if (po_.mode=="ATF4")      fitterATF_ = new TrackFitterAlgoATF(false);
        else if (po_.mode=="ATF5") fitterATF_ = new TrackFitterAlgoATF(true);
        else if (po_.mode=="PCA4") fitterLin_ = new TrackFitterAlgoLinearized();
        else if (po_.mode=="RET")  fitterRetina_ = new TrackFitterAlgoRetina();
        
    }

    // Destructor
    ~TrackFitter() {
        if (fitterLin_)    delete fitterLin_;
        if (fitterATF_)    delete fitterATF_;
        if (fitterRetina_) delete fitterRetina_;
    }


    // Setters
    void setNEvents(long long n)    { if (n != -1)  nEvents_     = n > 0 ? n : 0; }
    void setMaxCombinations(int n)  { if (n != -1)  maxCombs_    = n > 0 ? n : 0; }
    void setMaxTracks(int n)        { if (n != -1)  maxTracks_   = n > 0 ? n : 0; }
    void setVerbosity(int v)        { verbose_ = v; }

    // Getters
    // none

    // Main driver
    int run(TString src, TString out);


  private:
    // Member functions
    int makeTracks(TString src, TString out);

    // Configurations
    const TrackFitterOption po_;
    const TString prefixRoad_;
    const TString prefixTrack_;
    const TString suffix_;

    // Program options
    long long nEvents_;
    int maxCombs_;   // max number of combinations per road
    int maxTracks_;  // max number of tracks per event
    int verbose_;

    // Track fitters
    TrackFitterAlgoLinearized * fitterLin_;
    TrackFitterAlgoATF *        fitterATF_;
    TrackFitterAlgoRetina *     fitterRetina_;
};

#endif
