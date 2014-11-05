#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"
#include <stdint.h>
#include <vector>
#include <iostream>

typedef uint16_t count_type;
typedef uint32_t id_type;


#ifdef __CINT__
#pragma link C++ class vector<vector<float> >+;
#pragma link C++ class vector<vector<int> >+;
#pragma link C++ class vector<vector<id_type> >+;
#endif


int readRoads()
{
    gROOT->ProcessLine("#include <vector>");

    // Enter your file name
    TFile *tfile = new TFile("/uscms_data/d2/jiafu/L1TrackTrigger/CRAB_amsim/roads.root");

    // Enter your TTree name
    TTree *ttree = (TTree *) tfile->Get("ntupler/tree");
    TString prefix = "AMTTRoads_";
    TString suffix = "";

    // Setup vectors for the branches of interest
    std::vector<count_type> *               vr_nSuperstrips     = 0;
    std::vector<id_type> *                  vr_bankIndex        = 0;
    std::vector<std::vector<float> > *      vr_hitRs            = 0;
    std::vector<std::vector<float> > *      vr_hitPhis          = 0;
    std::vector<std::vector<float> > *      vr_hitZs            = 0;
    std::vector<std::vector<float> > *      vr_hitRErrors       = 0;
    std::vector<std::vector<float> > *      vr_hitPhiErrors     = 0;
    std::vector<std::vector<float> > *      vr_hitZErrors       = 0;
    std::vector<std::vector<float> > *      vr_hitClusWidths    = 0;
    std::vector<std::vector<float> > *      vr_hitStubWidths    = 0;
    std::vector<std::vector<id_type> > *    vr_hitSuperstripIds = 0;
    std::vector<std::vector<int> > *        vr_hitTrkIds        = 0;

    // Setup the TTree branches to those vectors
    ttree->SetBranchAddress(prefix + "nSuperstrips"      + suffix, &(vr_nSuperstrips));
    ttree->SetBranchAddress(prefix + "bankIndex"         + suffix, &(vr_bankIndex));
    ttree->SetBranchAddress(prefix + "hitRs"             + suffix, &(vr_hitRs));
    ttree->SetBranchAddress(prefix + "hitPhis"           + suffix, &(vr_hitPhis));
    ttree->SetBranchAddress(prefix + "hitZs"             + suffix, &(vr_hitZs));
    ttree->SetBranchAddress(prefix + "hitRErrors"        + suffix, &(vr_hitRErrors));
    ttree->SetBranchAddress(prefix + "hitPhiErrors"      + suffix, &(vr_hitPhiErrors));
    ttree->SetBranchAddress(prefix + "hitZErrors"        + suffix, &(vr_hitZErrors));
    ttree->SetBranchAddress(prefix + "hitClusWidths"     + suffix, &(vr_hitClusWidths));
    ttree->SetBranchAddress(prefix + "hitStubWidths"     + suffix, &(vr_hitStubWidths));
    ttree->SetBranchAddress(prefix + "hitSuperstripIds"  + suffix, &(vr_hitSuperstripIds));
    ttree->SetBranchAddress(prefix + "hitTrkIds"         + suffix, &(vr_hitTrkIds));

    // Number of events to read
    //unsigned nevents = 10;
    unsigned nevents = ttree->GetEntries();

     // Loop over events
    for (unsigned ievt=0; ievt<nevents; ++ievt) {
        // Get a new event
        ttree->GetEntry(ievt);

        // Number of roads
        const unsigned nroads = vr_hitSuperstripIds->size();
        std::cout << "Event " << ievt << " has " << nroads << " roads." << std::endl;

        // Loop over the roads
        for (unsigned i=0, j=0; i<nroads; ++i) {
            // Number of stubs
            const unsigned nstubs = vr_hitSuperstripIds->at(i).size();
            const unsigned nsuperstrips = vr_nSuperstrips->at(i);
            std::cout << "  - Road " << i << " has " << nstubs << " stubs in " << nsuperstrips << " superstrips." << std::endl;

            // Group by superstrip id
            std::vector<std::vector<unsigned> > stubs_by_ssId;
            id_type ssId=0, old_ssId=0;

            for (j=0; j<nstubs; ++j) {
                ssId = vr_hitSuperstripIds->at(i).at(j);
                std::cout << "      - Stub " << j << " ssId: " << ssId << " r,phi,z: " << vr_hitRs->at(i).at(j) << "," << vr_hitPhis->at(i).at(j) << "," << vr_hitZs->at(i).at(j) << std::endl;

                if (j == 0 || ssId != old_ssId) {
                    stubs_by_ssId.push_back(std::vector<unsigned>());
                }
                stubs_by_ssId.back().push_back(j);
                old_ssId = ssId;
            }

            //for (unsigned j=0; j<stubs_by_ssId.size(); ++j) {
            //    std::cout << "      - Superstrip " << j << " has " << stubs_by_ssId.at(j).size() << " stubs." << std::endl;
            //}

            // Make combinations
            std::vector<unsigned> indices;
            int ii, ij;
            for (ii=0; ii<int(nsuperstrips); ++ii)
                indices.push_back(0);

            j = 0;
            while (true) {
                std::cout << "      - Stub Comb " << j << " consists of: ";
                for (ii=0; ii<int(nsuperstrips); ++ii)
                    std::cout << stubs_by_ssId.at(ii).at(indices[ii]) << " ";
                std::cout << std::endl;

                for (ii=nsuperstrips-1; ii>=0; --ii) {
                    if (indices[ii] != stubs_by_ssId.at(ii).size() - 1)
                        break;
                }
                if (ii == -1)  break;

                indices[ii] += 1;
                for (ij = ii+1; ij<int(nsuperstrips); ++ij) {
                    indices[ij] = 0;
                }
                ++j;
            }
        }

        std::cout << std::endl;
    }

    // Close the file
    tfile->Close();

    return 0;
}
