#ifndef AMSimulation_TrackFitter_h_
#define AMSimulation_TrackFitter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/ProgramOption.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoPCA.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoATF.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoLTF.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/CombinationFactory.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/GhostBuster.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/DuplicateRemoval.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/MCTruthAssociator.h"
using namespace slhcl1tt;


class TrackFitter {

  public:
    // Constructor
    TrackFitter(const ProgramOption& po)
    : po_(po),
      nEvents_(po.maxEvents), verbose_(po.verbose),
      prefixRoad_("AMTTRoads_"), prefixTrack_("AMTTTracks_"), suffix_("") {

        // Decide the track fitter to use
        fitter_ = 0;
        if (po.algo == "PCA4" || po.algo == "PCA5") {
            fitter_ = new TrackFitterAlgoPCA(po);
        } else if (po.algo == "ATF4") {
            fitter_ = new TrackFitterAlgoATF(false);
        } else if (po.algo == "ATF5") {
            fitter_ = new TrackFitterAlgoATF(true);
        } else if (po.algo == "LTF") {
            fitter_ = new TrackFitterAlgoLTF(po);
        } else {
            throw std::invalid_argument("unknown track fitter algo.");
        }
    }

    // Destructor
    ~TrackFitter() {
        if (fitter_)  delete fitter_;
    }

    // Main driver
    int run();


  private:
    // Member functions
    int makeTracks(TString src, TString out);

    // Program options
    const ProgramOption po_;
    long long nEvents_;
    int verbose_;

    // Configurations
    const TString prefixRoad_;
    const TString prefixTrack_;
    const TString suffix_;

    // Track fitter
    TrackFitterAlgoBase *  fitter_;

    // Combination factory
    CombinationFactory combinationFactory_;

    // Ghost buster
    GhostBuster ghostBuster_;

    // MC truth associator
    MCTruthAssociator truthAssociator_;
};

#endif
