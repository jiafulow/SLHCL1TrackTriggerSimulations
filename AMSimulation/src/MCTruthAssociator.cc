#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/MCTruthAssociator.h"
using namespace slhcl1tt;

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <TMath.h>
#define  debug 0
#define  tt27_cuts 0

namespace {
// Comparator
bool sortByPt(const TrackingParticle& lhs, const TrackingParticle& rhs) {
    return (1.0/std::fabs(lhs.invPt)) > (1.0/std::fabs(rhs.invPt));
}

bool sortForSynEff(const TTTrack2& ltk, const TTTrack2& rtk){
  return (ltk.ndof() > rtk.ndof()) || (ltk.ndof() == rtk.ndof() && ltk.pt() > rtk.pt());
}

bool sortByMatchQuality(const std::pair<unsigned, float>& lhs, const std::pair<unsigned, float>& rhs) {
    return lhs.second < rhs.second;  // smaller is better
}

float fabsDiff(float lhs, float rhs) {
    return std::fabs(lhs - rhs);
}

float squaredNormDiff(float lhs, float rhs, float scale) {
    return std::pow((lhs - rhs)/scale, 2.0);
}

float resolution(float qbpT, std::string trk_param){
  float Const=1, G_Const=1, G_Media=1, G_Sigma=1, r=1;

  if(trk_param == "invPt"){
    Const   =  0.001702;
    G_Const = -0.001517;
    G_Media =  0.000256;
    G_Sigma =  0.148572;
  }
  else if(trk_param == "phi0"){
    Const   =  0.000663;
    G_Const = -0.000559;
    G_Media = -0.000678;
    G_Sigma =  0.155241;
  }
  else if(trk_param == "z0"){
    Const   =  0.178339;
    G_Const = -0.099997;
    G_Media =  0.009426;
    G_Sigma =  0.838881;
  }
  else if(trk_param == "cottheta"){
    Const   =  0.003157;
    G_Const = -0.001072;
    G_Media =  0.005015;
    G_Sigma =  0.268481;
  }
  else
    std::cout<<"Parameter not recognized!"<<std::endl;

  r = Const + G_Const*TMath::Gaus(qbpT, G_Media, G_Sigma);
  return r;
}

  static const float degrees_of_freedom = 4.0;
  static const float match_chi2_cut   = 12.8;


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

    //rms_invPt_    = 0.001283;
    //rms_phi0_     = 0.000453;
    //rms_cottheta_ = 0.002403;
    //rms_z0_       = 0.083406;
    //rms_d0_       = 1.000000;
}

// _____________________________________________________________________________
void MCTruthAssociator::associate(std::vector<TrackingParticle>& trkParts, std::vector<TTTrack2>& tracks) {

    // Sort tracking particles by pT
    std::sort(trkParts.begin(), trkParts.end(), sortByPt);

    // Sort by Logic and pT combined
    std::sort(tracks.begin(), tracks.end(), sortForSynEff);
    
    const unsigned nparts  = trkParts.size();
    const unsigned ntracks = tracks.size();

    // Create the map for matching
    //std::map<unsigned, std::vector<std::pair<unsigned, float> > > matches;  // key=ipart, value=vector of (itrack, quality) pair

    // Loop on all tracking particles
    //for (unsigned ipart=0; ipart<nparts; ++ipart) {
    //    const TrackingParticle& trkPart = trkParts.at(ipart);
    //    assert(trkParts.at(ipart).tpId >= 0);

    //    std::vector<std::pair<unsigned, float> >& ipart_matches = matches[ipart];
    //    assert(ipart_matches.size() == 0);

        // Loop on all reconstructed tracks
    //    for (unsigned itrack=0; itrack<ntracks; ++itrack) {
    //        const TTTrack2& track = tracks.at(itrack);
    //        float quality = 1.e15;  // It will be now the match chi2 value

    //        if (accept(trkPart, track, quality)) {
    //            ipart_matches.push_back(std::make_pair(itrack, quality));
    //        }
    //    }
    //}

    // Debug
    //for (unsigned ipart=0; ipart<nparts; ++ipart) {
    //    const std::vector<std::pair<unsigned, float> >& ipart_matches = matches.at(ipart);
    //    for (unsigned imatch=0; imatch<ipart_matches.size(); ++imatch) {
    //        unsigned itrack = ipart_matches.at(imatch).first;
    //        float quality = ipart_matches.at(imatch).second;
    //        std::cout << "ipart: " << ipart << " itrack: " << itrack << " quality: " << quality << std::endl;
    //    }
    //}

    // Stores flags to each MC/AM track
    std::vector<int> mcCategories(nparts, ParticleCategory::NOTFOUND);
    std::vector<int> recoCategories(ntracks, TrackCategory::FAKE);

    // Loop on all tracking particles
    if(debug == 1) std::cout<<"\n\n"<<"----------- New Event -----------"<<std::endl;
    //if(debug == 1) std::cout<<"MC tracks: "<<nparts<<"\t\tAM tracks: "<<ntracks<<std::endl;
    for (unsigned ipart=0; ipart<nparts; ++ipart) {
	if(tt27_cuts == 1){
	  float Pt   = 1./fabs(trkParts.at(ipart).invPt);
	  float Phi0 = trkParts.at(ipart).phi0;
	  float Eta  = TMath::ASinH(trkParts.at(ipart).cottheta);
	  if(Pt < 3) std::cout<<"MC Track with Pt < 3GeV"<<std::endl; 
	  if(Phi0 < 3.14/4. || Phi0 > 3.14/2.) std::cout<<"MC Track out of Phi cuts"<<std::endl;
	  if(Eta < 0 || Eta > 0.733333) std::cout<<"MC Track out of Eta cuts"<<std::endl;
	}
	if(debug == 1) std::cout<<"["<<ipart<<"] Part_tpId --> "<<trkParts.at(ipart).tpId<<"\tPart_pdgId --> "<<trkParts.at(ipart).pdgId<<"\tPt --> "<<1./fabs(trkParts.at(ipart).invPt)<<std::endl;

        bool foundTheBest = false;
	float best_quality = match_chi2_cut, i_quality = -1;
	int best_match_id = -1;
        for (unsigned am_itrack=0; am_itrack<ntracks; ++am_itrack) {// Loop over reco tracks
	    //std::cout<<"AM Track Chi2/ndof(from FITTER): "<<tracks.at(am_itrack).chi2()/tracks.at(am_itrack).ndof()<<std::endl;

            // Compute our match chi2 definition for each combination and 
	    // checks if it is below our defined threshold (12.8)
	    bool accpt_quality = accept(trkParts.at(ipart), tracks.at(am_itrack), i_quality);

	    // Debug
	    if(debug == 1) std::cout<<"AM Track --> "<<am_itrack<<"\tLogic --> "<<tracks.at(am_itrack).ndof()<<"\tPt --> "<<tracks.at(am_itrack).pt()<<"\tQuality --> "<<i_quality<<"\tCat: "<<recoCategories.at(am_itrack)<<std::endl;

	    // If matchChi2 > 12.8 keeps the FAKE flag for AM track
	    if(!accpt_quality) continue;

            // Debug (checking i_quality dump)
            //std::cout<<"AM Track --> "<<am_itrack<<"\tLogic --> "<<tracks.at(am_itrack).ndof()<<"\tPt --> "<<tracks.at(am_itrack).pt()<<"\tQuality --> "<<i_quality<<"\tisDuplicate --> "<<tracks.at(am_itrack).isDuplicate()<<std::endl;

	    // If AM track is not marked as good track, then categorize the track
	    if(recoCategories.at(am_itrack) != TrackCategory::GOOD){
            
		// Any AM track at this stage is below the match chi2 cut
		// then it will be good or duplicate
		recoCategories.at(am_itrack) = TrackCategory::DUPLICATE;
                tracks.at(am_itrack).setTpId(trkParts.at(ipart).tpId);

		// Find the AM track that best matches with the real track
		if(i_quality < best_quality){
  		   best_match_id = am_itrack;
		   best_quality  = i_quality;
                   foundTheBest  = true;
		}
            }
	}// End loop over AM tracks            
	    
	if(foundTheBest){
           //if(debug == 1) std::cout<<"bestMatchId: "<<best_match_id<<std::endl;
           mcCategories.at(ipart) = ParticleCategory::FOUND;
           tracks.at(best_match_id).setMatchChi2(best_quality);
           tracks.at(best_match_id).setTpId(trkParts.at(ipart).tpId);
           recoCategories.at(best_match_id) = TrackCategory::GOOD;
	}

    }// End loop over tracking particles

    
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
        tracks.at(itrack).setSynTpId(cat);

        if (cat == TrackCategory::FAKE)
          ++nfakes;
        else if (cat == TrackCategory::DUPLICATE)
          ++nduplicates;
        else
          ++ngoods;
    }
    
    // Debug
    if(debug == 1) std::cout<<"Found --> "<<nfounds<<"\tNotFond --> "<<nnotfounds<<std::endl;
    if(debug == 1) std::cout<<"Good --> "<<ngoods<<"\tDuplicates --> "<<nduplicates<<"\tFakes --> "<<nfakes<<std::endl;
    assert(nfounds + nnotfounds == nparts);
    assert(ngoods + nduplicates + nfakes == ntracks);
    assert(nfounds == ngoods);
}

// _____________________________________________________________________________
bool MCTruthAssociator::accept(const TrackingParticle& trkPart, const TTTrack2& track, float& quality){

    quality = (squaredNormDiff(trkPart.invPt   , track.invPt()   , resolution(track.invPt(),"invPt") ) +
               squaredNormDiff(trkPart.phi0    , track.phi0()    , resolution(track.invPt(),"phi0") ) +
               squaredNormDiff(trkPart.cottheta, track.cottheta(), resolution(track.invPt(),"cottheta") ) +
               squaredNormDiff(trkPart.z0      , track.z0()      , resolution(track.invPt(),"z0") )
              )/degrees_of_freedom;

    bool acc = quality < match_chi2_cut;

    return acc;
}

// _____________________________________________________________________________
void MCTruthAssociator::print() {
    //std::cout << "rms invPt: " << rms_invPt_ << " phi0: " << rms_phi0_ << " cottheta: " << rms_cottheta_ << " z0: " << rms_z0_ << " d0: " << rms_d0_ << std::endl;
}
