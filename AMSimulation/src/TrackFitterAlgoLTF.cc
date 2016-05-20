#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoLTF.h"
using namespace slhcl1tt;

int TrackFitterAlgoLTF::fit(const TTRoadComb& acomb, TTTrack2& atrack) {
    // Arrange the stub coordinates in the format expected by the fitter
    std::vector<double> vars;
    for (unsigned istub=0; istub<acomb.stubs_phi.size(); ++istub) {
        vars.push_back(acomb.stubs_phi.at(istub));
        vars.push_back(acomb.stubs_r.at(istub));
        vars.push_back(acomb.stubs_z.at(istub));
    }

    double normChi2 = linearizedTrackFitter_->fit(vars, acomb.hitBits);
    const std::vector<double>& pars = linearizedTrackFitter_->estimatedPars();
    //const std::vector<double>& principals = linearizedTrackFitter_->principalComponents();
    const std::vector<double>& principals = linearizedTrackFitter_->normalizedPrincipalComponents();

    // TODO: This should be returned by the LinearizedTrackFitter
    int ndof = 8;
    if (1 <= acomb.hitBits && acomb.hitBits <= 6) {
      ndof -= 2;
    }

    atrack.setTrackParams(0.003 * 3.8 * pars[0], pars[1], pars[2], pars[3], 0, normChi2*ndof, ndof, 0, 0);

    std::vector<float> principals_vec;
    for (unsigned ivar=0; ivar<principals.size(); ++ivar) {
        principals_vec.push_back(principals.at(ivar));
    }
    atrack.setPrincipals(principals_vec);

    return 0;
}
