#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitter.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackWriter.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterDas.h"

#include "TFileCollection.h"

int TrackFitter::readRoads(TString inFile)
{
  chain_ = new TChain("ntupler/tree");

  if (inFile.EndsWith(".root")) 
  {
    if (chain_->Add(inFile))
    {
      if (verbose_) std::cout<<Info()<<"Successfully read "<<inFile<<std::endl;
    }
    else 
    {
      std::cout<<Error()<<"Failed to read "<<inFile<<std::endl;
      return 1;
    }
  }
  else if (inFile.EndsWith(".txt")) 
  {
    TFileCollection fc("fileinfolist", "", inFile);
    if (chain_->AddFileInfoList((TCollection*)fc.GetList())) 
    {
      if (verbose_) std::cout<<Info()<<"Successfully read "<<inFile<<std::endl;
    } 
    else 
    {
      std::cout<<Error()<<"Failed to read "<<inFile<<std::endl;
      return 1;
    }
  }
  else
  {
    std::cout<<Error()<<"Input source must be either .root or .txt"<<std::endl;
    return 1;
  }
  return 0;
}

int TrackFitter::makeTracks(TString outFile) 
{
  if (verbose_) std::cout<<Info()<<"Reading "<<nEvents_<<" events and fitting tracks."<<std::endl;

  std::vector<count_type>           *vr_nSuperstrips      = 0;    
  std::vector<std::vector<float> >  *vr_hitRs             = 0;    
  std::vector<std::vector<float> >  *vr_hitPhis           = 0;    
  std::vector<std::vector<float> >  *vr_hitZs             = 0;    
  std::vector<std::vector<float> >  *vr_hitRErrors        = 0;    
  std::vector<std::vector<float> >  *vr_hitPhiErrors      = 0;    
  std::vector<std::vector<float> >  *vr_hitZErrors        = 0;    
  std::vector<std::vector<int> >    *vr_hitClusWidths     = 0;    
  std::vector<std::vector<float> >  *vr_hitStubWidths     = 0;    
  
  chain_->SetBranchAddress(prefixRoad_ + "nSuperstrips"      + suffix_, &(vr_nSuperstrips));
  chain_->SetBranchAddress(prefixRoad_ + "hitRs"             + suffix_, &(vr_hitRs));
  chain_->SetBranchAddress(prefixRoad_ + "hitPhis"           + suffix_, &(vr_hitPhis));
  chain_->SetBranchAddress(prefixRoad_ + "hitZs"             + suffix_, &(vr_hitZs));
  chain_->SetBranchAddress(prefixRoad_ + "hitRErrors"        + suffix_, &(vr_hitRErrors));
  chain_->SetBranchAddress(prefixRoad_ + "hitPhiErrors"      + suffix_, &(vr_hitPhiErrors));
  chain_->SetBranchAddress(prefixRoad_ + "hitZErrors"        + suffix_, &(vr_hitZErrors));
  chain_->SetBranchAddress(prefixRoad_ + "hitClusWidths"     + suffix_, &(vr_hitClusWidths));
  chain_->SetBranchAddress(prefixRoad_ + "hitStubWidths"     + suffix_, &(vr_hitStubWidths));
  
  // For writing
  if (verbose_)  std::cout<<Info()<<"Recreating "<<outFile<<std::endl;
  TTTrackWriter writer(chain_, outFile, prefixTrack_, suffix_);
  
  // Kind of Fitter is set here
  TrackFitterDas trackFitter;
		
	// Loop over all events
  int nPassed=0, nKept=0;
  for (long long ievt=0; ievt<nEvents_; ++ievt) 
	{
	  Long64_t local_entry=chain_->LoadTree(ievt);
    if (local_entry<0) break;
    chain_->GetEntry(ievt);

    const unsigned nroads=vr_nSuperstrips->size();
    if (verbose_>1 && ievt%5000==0)  std::cout<<Debug()<<Form("... Processing event: %7lld, keeping: %7i, fitting: %7i", ievt, nKept, nPassed) << std::endl;
    if (verbose_>2)  std::cout<<Debug()<<"... evt: "<<ievt<<" # roads: "<<nroads<<std::endl;
		
		std::vector<TTTrack> tracks;
    tracks.reserve(200);
		for (unsigned int i=0; i<nroads; ++i) 
		{
		  TTTrack *track=trackFitter.fit(vr_hitPhis->at(i), vr_hitRs->at(i), vr_hitZs->at(i));
      track->setRoadIndex(i);
			tracks.push_back(*track);
      if ((int)tracks.size()>=maxTracks_) break;
		}
		
		if (!tracks.empty()) ++nPassed;
    writer.fill(tracks); //, genParts);
		++nKept;
  }

  if (verbose_)  std::cout << Info() << "Processed " << nEvents_ << " events, kept " << nKept << ", fitted " << nPassed << std::endl;

  long long nentries = writer.write();
  assert(nentries == nKept);
	
  return 0;
}

// _____________________________________________________________________________
// Main driver
int TrackFitter::run(TString src, TString out) {
    gROOT->ProcessLine("#include <vector>");  // how is it not loaded?

    int exitcode = 0;
    Timing(1);

    exitcode = readRoads(src);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = makeTracks(out);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
