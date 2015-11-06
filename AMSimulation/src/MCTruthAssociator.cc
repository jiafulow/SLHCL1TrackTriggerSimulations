#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/MCTruthAssociator.h"
using namespace slhcl1tt;

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>


namespace {
// Comparator
bool sortByPt(const TrackingParticle& lhs, const TrackingParticle& rhs) {
    return (1.0/std::abs(lhs.invPt)) > (1.0/std::abs(rhs.invPt));
}

bool sortByMatchQuality(const std::pair<unsigned, float>& lhs, const std::pair<unsigned, float>& rhs) {
    return lhs.second < rhs.second;  // smaller is better
}

float absDiff(float lhs, float rhs) {
    return std::abs(lhs - rhs);
}

float squaredNormDiff(float lhs, float rhs, float scale) {
    return std::pow((lhs - rhs)/scale, 2.0);
}
}

// _____________________________________________________________________________
MCTruthAssociator::MCTruthAssociator() {
    //rms_invPt_    = 0.000165559;
    //rms_phi0_     = 7.29527e-05;
    //rms_cottheta_ = 0.00215212 ;
    //rms_z0_       = 0.0819977  ;
    //rms_d0_       = 1.00000    ;

    //rms_invPt_    = 0.002759;
    //rms_phi0_     = 0.000646;
    //rms_cottheta_ = 0.002695;
    //rms_z0_       = 0.08586 ;
    //rms_d0_       = 1.00000 ;

    rms_invPt_    = 0.001283;
    rms_phi0_     = 0.000453;
    rms_cottheta_ = 0.002403;
    rms_z0_       = 0.083406;
    rms_d0_       = 1.000000;
}

// _____________________________________________________________________________
void MCTruthAssociator::associate(std::vector<TrackingParticle>& trkParts, std::vector<TTTrack2>& tracks) {

    // Sort tracking particles by pT
    std::sort(trkParts.begin(), trkParts.end(), sortByPt);

    const unsigned nparts  = trkParts.size();
    const unsigned ntracks = tracks.size();

    // Create the map for matching
    std::map<unsigned, std::vector<std::pair<unsigned, float> > > matches;  // key=ipart, value=vector of (itrack, quality) pair

    // Loop on all tracking particles
    for (unsigned ipart=0; ipart<nparts; ++ipart) {
        const TrackingParticle& trkPart = trkParts.at(ipart);
        assert(trkParts.at(ipart).tpId >= 0);

        std::vector<std::pair<unsigned, float> >& ipart_matches = matches[ipart];
        assert(ipart_matches.size() == 0);

        // Loop on all reconstructed tracks
        for (unsigned itrack=0; itrack<ntracks; ++itrack) {
            const TTTrack2& track = tracks.at(itrack);
            float quality = 999.;  // smaller is better

            if (accept(trkPart, track, quality)) {
                ipart_matches.push_back(std::make_pair(itrack, quality));
            }
        }
    }

    // Debug
    //for (unsigned ipart=0; ipart<nparts; ++ipart) {
    //    const std::vector<std::pair<unsigned, float> >& ipart_matches = matches.at(ipart);
    //    for (unsigned imatch=0; imatch<ipart_matches.size(); ++imatch) {
    //        unsigned itrack = ipart_matches.at(imatch).first;
    //        float quality = ipart_matches.at(imatch).second;
    //        std::cout << "ipart: " << ipart << " itrack: " << itrack << " quality: " << quality << std::endl;
    //    }
    //}

    std::vector<int> mcCategories(nparts, ParticleCategory::NOTFOUND);
    std::vector<int> recoCategories(ntracks, TrackCategory::FAKE);

    // Loop on all tracking particles
    for (unsigned ipart=0; ipart<nparts; ++ipart) {
        std::vector<std::pair<unsigned, float> >& ipart_matches = matches.at(ipart);
        std::sort(ipart_matches.begin(), ipart_matches.end(), sortByMatchQuality);

        if (ipart_matches.size() == 0) {
            // Do nothing

        } else {
            bool foundTheBest = false;
            for (unsigned imatch=0; imatch<ipart_matches.size(); ++imatch) {
                unsigned itrack = ipart_matches.at(imatch).first;

                //if (recoCategories.at(itrack) == TrackCategory::GOOD)
                //    continue;
                if (recoCategories.at(itrack) >= 0)
                    continue;

                if (!foundTheBest) {
                    mcCategories.at(ipart) = ParticleCategory::FOUND;
                    //recoCategories.at(itrack) = TrackCategory::GOOD;
                    recoCategories.at(itrack) = trkParts.at(ipart).tpId;
                    foundTheBest = true;

                } else {
                    mcCategories.at(ipart) = ParticleCategory::FOUND;
                    recoCategories.at(itrack) = TrackCategory::DUPLICATE;
                }

                // Debug
                //std::cout << "ipart: " << ipart << " itrack: " << itrack << " quality: " << ipart_matches.at(imatch).second << " chi2Red: " << tracks.at(itrack).chi2()/tracks.at(itrack).ndof() << " ndof: " << tracks.at(itrack).ndof() << " patternRef: " << tracks.at(itrack).patternRef() << " mcCategory: " << mcCategories.at(ipart) << " recoCategoy: " << recoCategories.at(itrack) << std::endl;
            }
        }
    }

    // Sanity check
    unsigned ngoods = 0, nduplicates = 0, nfakes = 0, nfounds = 0, nnotfounds = 0;

    for (unsigned ipart=0; ipart<nparts; ++ipart) {
        const int cat = mcCategories.at(ipart);
        if (cat == ParticleCategory::NOTFOUND)
            ++nnotfounds;
        else
            ++nfounds;
    }

    for (unsigned itrack=0; itrack<ntracks; ++itrack) {
        const int cat = recoCategories.at(itrack);
        if (cat == TrackCategory::FAKE)
            ++nfakes;
        else if (cat == TrackCategory::DUPLICATE)
            ++nduplicates;
        else
            ++ngoods;

        tracks.at(itrack).setSynTpId(cat);
    }

    assert(nfounds + nnotfounds == nparts);
    assert(ngoods + nduplicates + nfakes == ntracks);
    assert(nfounds == ngoods);
}

// _____________________________________________________________________________
bool MCTruthAssociator::accept(const TrackingParticle& trkPart, const TTTrack2& track, float& quality) {
    static const float sigma = 5.;
    bool acc =  (absDiff(trkPart.invPt   , track.invPt()   ) < (rms_invPt_    * sigma)) &&
                (absDiff(trkPart.phi0    , track.phi0()    ) < (rms_phi0_     * sigma)) &&
                (absDiff(trkPart.cottheta, track.cottheta()) < (rms_cottheta_ * sigma)) &&
                (absDiff(trkPart.z0      , track.z0()      ) < (rms_z0_       * sigma)) &&
                (absDiff(trkPart.d0      , track.d0()      ) < (rms_d0_       * sigma));

    quality = squaredNormDiff(trkPart.invPt   , track.invPt()   , rms_invPt_   ) +
              squaredNormDiff(trkPart.phi0    , track.phi0()    , rms_phi0_    ) +
              squaredNormDiff(trkPart.cottheta, track.cottheta(), rms_cottheta_) +
              squaredNormDiff(trkPart.z0      , track.z0()      , rms_z0_      ) +
              squaredNormDiff(trkPart.d0      , track.d0()      , rms_d0_      );
    return acc;
}

// _____________________________________________________________________________
void MCTruthAssociator::print() {
    std::cout << "rms invPt: " << rms_invPt_ << " phi0: " << rms_phi0_ << " cottheta: " << rms_cottheta_ << " z0: " << rms_z0_ << " d0: " << rms_d0_ << std::endl;
}
