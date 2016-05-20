#ifndef AMSimulation_TrackFitterAlgoLTF_h_
#define AMSimulation_TrackFitterAlgoLTF_h_

#include <memory>
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/ProgramOption.h"
#include "LinearizedTrackFit/LinearizedTrackFit/interface/LinearizedTrackFitter.h"

namespace slhcl1tt {

// The second parameter in the LinearizedTrackFitter constructor specifies if the fitter will replace the R coordinate of stubs in 2S modules in the disks
// with the extrapolated R using a tan(theta) pre-estimate.
class TrackFitterAlgoLTF : public TrackFitterAlgoBase {
   public:
  TrackFitterAlgoLTF(const slhcl1tt::ProgramOption& po) :
    TrackFitterAlgoBase(),
    linearizedTrackFitter_(std::make_shared<LinearizedTrackFitter>("LinearizedTrackFit/LinearizedTrackFit/python/ConstantsProduction/", true, true)),
    verbose_(po.verbose)
  {}

    ~TrackFitterAlgoLTF() {}

    int fit(const TTRoadComb& acomb, TTTrack2& atrack);

  private:
    std::shared_ptr<LinearizedTrackFitter> linearizedTrackFitter_;
    int verbose_;
};

}  // namespace slhcl1tt

#endif
