#ifndef AMSimulation_MCTruthAssociator_h_
#define AMSimulation_MCTruthAssociator_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TrackingParticle.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack2.h"
#include <vector>

namespace slhcl1tt {

// Implement Luciano's synthetic definition (May 12, 2015)
// =======================================================
// Possible categories include:
// -    For input tracks: found, not found
// -    For output tracks: good, fake, duplicate
//
// i.   Start with two sets of tracks: the "input" set from the MC truth and
//      the "output" set from the list of tracks reconstructed by the
//      integrated simulation.
// ii.  For each possible combination of one input track and one output track,
//      we plot the difference of the four parameters (phi, 1/Pt, z0, cotTheta).
//      Hopefully these four plots will show a peak around 0 and we can set
//      reasonably loose cuts (3-4 sigmas)
// iii. Loop on all true tracks. For each input track search the output tracks
//      and count how many fall within the cuts on all four parameters and are
//      not marked as "good" (see below)
//        0: do nothing
//        1: mark the input track "found" and mark output track "good",
//      > 1: mark the input track "found", mark the best matching output track
//           "good", mark the remaining matching output tracks as "duplicate".
//      Note that by construction #reconstructed = #good + #duplicates + #fakes
//      and #found = #good

class MCTruthAssociator {
  public:
    // Constructor
    MCTruthAssociator();

    // Destructor
    ~MCTruthAssociator() {}

    // Enum
    enum ParticleCategory { NOTFOUND=-1, FOUND=1 };
    enum TrackCategory    { FAKE=-2, DUPLICATE=-1, GOOD=1 };

    void associate(std::vector<TrackingParticle>& trkParts, std::vector<TTTrack2>& tracks);

    bool accept(const TrackingParticle& trkPart, const TTTrack2& track, float& quality);

    // Debug
    void print();

  private:

    // Member data
    //float rms_invPt_;
    //float rms_phi0_;
    //float rms_cottheta_;
    //float rms_z0_;
    //float rms_d0_;

};

}

#endif
