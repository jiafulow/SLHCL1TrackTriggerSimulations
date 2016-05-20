#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/DuplicateRemoval.h"
using namespace slhcl1tt;

#include <vector>
#include <algorithm>
#include <cassert>
#include <exception>


///Just to sort by logic
namespace{

  bool sortForDuplicateRm(const TTTrack2& ltk, const TTTrack2& rtk){
    return (ltk.ndof() > rtk.ndof()) || (ltk.ndof() == rtk.ndof() && ltk.pt() > rtk.pt());
  }

}



void DuplicateRemoval::CheckTracks(std::vector<TTTrack2>& full_am_track_list, int dupRm){

  // Prevents wrong checking
  if(dupRm > 6){
    std::cout<<"!! ERROR: Number of stubs above maximum (6) !!"<<std::endl;
    throw std::exception();
  }

  // If setted, duplicate removal is done
  else if(dupRm != -1 && dupRm < 7){
    ///Sort AM tracks by logic and pt (decreasing)
    std::sort(full_am_track_list.begin(), full_am_track_list.end(), sortForDuplicateRm);

  
    ///The duplicate removal itself
    std::vector<TTTrack2> unique_tracks;
    bool duplicate_found;
    for(unsigned int itrack = 0; itrack < full_am_track_list.size(); itrack++){
      //if(full_am_track_list.at(itrack).chi2()/full_am_track_list.at(itrack).ndof() > 3) std::cout<<"Violates Chi2/ndof<3 cut"<<std::endl;

      duplicate_found = false;
      ///Always accept the first track
      if(unique_tracks.size() == 0) unique_tracks.push_back( full_am_track_list.at(itrack) );
      else{
        unsigned int nstubs = full_am_track_list.at(itrack).stubRefs().size();
        for(unsigned int jtrack = 0; jtrack < unique_tracks.size(); jtrack++){
	  //Just a sanity check
	  unsigned int j_nstubs = unique_tracks.at(jtrack).stubRefs().size();
  	  assert(nstubs == j_nstubs);
      
	  int sharedStubs = 0;
	  for(unsigned int istub = 0; istub < nstubs; istub++){
	    //When comparing two 5/6's tracks we don't consider the pedestal value as a stub reference
	    if( full_am_track_list.at(itrack).stubRefs()[istub] == 999999999 && unique_tracks.at(jtrack).stubRefs()[istub] == 999999999 ) continue;

	    //Compares the stubs in each layer
	    if( full_am_track_list.at(itrack).stubRefs()[istub] == unique_tracks.at(jtrack).stubRefs()[istub] ) sharedStubs++;
	  }
	  if(sharedStubs > dupRm) duplicate_found = true;
        }//loop over non-duplicate tracks (unique_tracks vector)

        //If track is not sharing more than allowed number of stubs, store it
        if(!duplicate_found)
          unique_tracks.push_back( full_am_track_list.at(itrack) );

      }//else end
    }//loop over all AM tracks


    //Remove the duplicate tracks from the original list
    //And keeps only the unique tracks
    full_am_track_list.resize(unique_tracks.size());
    full_am_track_list = unique_tracks;
    assert(full_am_track_list.size() == unique_tracks.size());
  }

  else{
    // The standard situation... no duplicate removal
    // So, nothing is done even calling the duplication removal member
  }
  
  return;
}
