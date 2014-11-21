#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitter.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackReader.h"


// _____________________________________________________________________________
// Do track fitting
int TrackFitter::makeTracks(TString src, TString out) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and fitting tracks." << std::endl;

    // _________________________________________________________________________
    // For reading
    TTRoadReader reader(verbose_);

    if (reader.init(src, prefixRoad_, suffix_)) {
        std::cout << Error() << "Failed to initialize TTRoadReader." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // For writing
    TTTrackWriter writer(verbose_);
    if (writer.init(reader.getChain(), out, prefixTrack_, suffix_)) {
        std::cout << Error() << "Failed to initialize TTTrackWriter." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // Loop over all events

    // Bookkeepers
    int nPassed = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nroads = reader.vr_bankIndex->size();
        if (verbose_>1 && ievt%5000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, fitting: %7i", ievt, nKept, nPassed) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # roads: " << nroads << std::endl;

        if (!nroads) {  // skip if no road
            writer.fill(std::vector<TTTrack2>());
            ++nKept;
            continue;
        }

        // _____________________________________________________________________
        // Arrange track fit combinations

        std::vector<TTTrack2> tracks;
        tracks.reserve(200);

        // Loop over the roads
        for (unsigned i=0, j=0; i<nroads; ++i) {
            // # superstrips, # stubs
            const unsigned nsuperstrips = reader.vr_nsuperstrips->at(i);
            const unsigned nstubs = reader.vr_stubRefs->at(i).size();

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << i << " # superstrips: " << nsuperstrips << " # stubs: " << nstubs <<  std::endl;

            // Group by superstrip id
            std::vector<std::vector<unsigned> > stubRefsGrouped;
            unsigned ref = 0;
            id_type ssId = 0, old_ssId = 0;

            for (j=0; j<nstubs; ++j) {
                ref = reader.vr_stubRefs->at(i).at(j);
                ssId = reader.vr_stubSuperstripIds->at(i).at(j);
                if (verbose_>2)  std::cout << Debug() << "... ... ... stub: " << j << " ssId: " << ssId << " ref: " << ref << std::endl;

                if (j == 0 || ssId != old_ssId) {
                    stubRefsGrouped.push_back(std::vector<unsigned>());
                }
                stubRefsGrouped.back().push_back(ref);
                old_ssId = ssId;
            }

            if (verbose_>3) {
                for (j=0; j<stubRefsGrouped.size(); ++j)
                    std::cout << Debug() << "... ... ... superstrip: " << j << " # stubRefs: " << stubRefsGrouped.at(j).size() << std::endl;
            }

            // Make combinations
            if (nsuperstrips == stubRefsGrouped.size()) {
                std::vector<unsigned> stubRefs;
                std::vector<unsigned> indices(nsuperstrips, 0);  // init to zeroes
                int ii, jj;

                j = 0;
                while (true) {
                    stubRefs.clear();
                    for (ii=0; ii<int(nsuperstrips); ++ii)
                        stubRefs.push_back(stubRefsGrouped.at(ii).at(indices[ii]));
                    tracks.emplace_back(i, j, stubRefs);
                    if ((int) tracks.size() >= maxTracks_)
                        break;

                    for (ii=nsuperstrips-1; ii>=0; --ii)
                        if (indices[ii] != stubRefsGrouped.at(ii).size() - 1)
                            break;  // take the last index that has not reached the end
                    if (ii == -1)  break;

                    indices[ii] += 1;  // increment that index
                    for (jj = ii+1; jj<int(nsuperstrips); ++jj)
                        indices[jj] = 0;  // set indices behind that index to zeroes

                    ++j;
                }

            } else {
                std::cout << Warning() << "Disagreement between nsuperstrips and stubRefsGrouped.size(): " << nsuperstrips << " vs " << stubRefsGrouped.size() << std::endl;
            }
        }

        // _____________________________________________________________________
        // Call track fitter on track candidates

        const unsigned ntracks = tracks.size();
        int status = 0;

        // Loop over the track candidates
        for (unsigned i=0, j=0; i<ntracks; ++i) {
            TTTrack2& track = tracks.at(i);
            const std::vector<unsigned>& stubRefs = track.stubRefs();

            if (verbose_>2) {
                std::cout << Debug() << "... ... track: " << i << " roadRef: " << track.roadRef() << " combRef: " << track.combRef() << " stubRefs: ";
                std::copy(stubRefs.begin(), stubRefs.end(), std::ostream_iterator<unsigned>(std::cout, " "));
                std::cout << std::endl;
            }

            std::vector<TTHit> hits;
            for (j=0; j<stubRefs.size(); ++j) {
                const unsigned& ref = stubRefs.at(j);
                hits.emplace_back(TTHit{                // using POD type constructor
                    reader.vb_r->at(ref),
                    reader.vb_phi->at(ref),
                    reader.vb_z->at(ref),
                    0.,
                    0.,
                    0.
                });
            }

            // Fit
            if (po.mode == 0)
                status = fitterLin_->fit(hits, track);
            else
                status = fitterDas_->fit(hits, track);

            if (verbose_>2)  std::cout << Debug() << "... ... track: " << i << " status: " << status << " ... " << std::endl;
        }

        // _____________________________________________________________________
        // Remove fails and duplicates

        // FIXME: implement this

        if (! tracks.empty())
            ++nPassed;

        writer.fill(tracks);
        ++nKept;
    }

    if (verbose_)  std::cout << Info() << "Processed " << nEvents_ << " events, kept " << nKept << ", fitted " << nPassed << std::endl;

    long long nentries = writer.writeTree();
    assert(nentries == nKept);

    return 0;
}


// _____________________________________________________________________________
// Main driver
int TrackFitter::run(TString src, TString out) {
    int exitcode = 0;
    Timing(1);

    exitcode = makeTracks(src, out);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
