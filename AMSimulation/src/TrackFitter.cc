#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitter.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackReader.h"

namespace {
// Combination arrangement
// groups[i][j] is the j-th element in the i-th group
// combinations[i][j] is the j-th element in the i-th combination
template<typename T>
std::vector<std::vector<T> > arrangeCombinations(const std::vector<std::vector<T> >& groups) {
    std::vector<T> combination;
    std::vector<std::vector<T> > combinations;

    const int ngroups = groups.size();
    std::vector<unsigned> indices(ngroups, 0);  // init to zeroes

    int i=0, j=0;
    while (true) {
        combination.clear();
        for (i=0; i<ngroups; ++i) {
            if (groups.at(i).size())
                combination.push_back(groups.at(i).at(indices.at(i)));
            else  // empty group
                combination.push_back(999999);
        }
        combinations.push_back(combination);

        for (i=ngroups-1; i>=0; --i)
            if (groups.at(i).size())
                if (indices.at(i) != groups.at(i).size() - 1)
                    break;  // take the last index that has not reached the end
        if (i == -1)  break;

        indices[i] += 1;  // increment that index
        for (j=i+1; j<ngroups; ++j)
            indices[j] = 0;  // set indices behind that index to zeroes
    }

    return combinations;
}
}


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

    // Containers
    std::vector<TTTrack2> tracks;
    tracks.reserve(300);

    // Bookkeepers
    long int nRead = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nroads = reader.vr_patternRef->size();
        if (verbose_>1 && ievt%1000==0)  std::cout << Debug() << Form("... Processing event: %7lld, fitting: %7ld", ievt, nKept) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # roads: " << nroads << std::endl;

        if (!nroads) {  // skip if no road
            writer.fill(std::vector<TTTrack2>());
            ++nRead;
            continue;
        }

        tracks.clear();
        int fitstatus = 0;

        switch (po_.mode) {
        case 2:
            // _________________________________________________________________
            // Track fitters taking the entire road

            // Loop over the roads
            for (unsigned iroad=0; iroad<nroads; ++iroad) {

                const unsigned tower = reader.vr_tower->at(iroad);
                const unsigned nstubs = reader.vr_nstubs->at(iroad);

                std::vector<unsigned> stubRefsAllLayers; // collect stubRefs from all layers
                for (unsigned isuperstrip=0; isuperstrip<reader.vr_stubRefs->at(iroad).size(); ++isuperstrip)
                    for (unsigned istub=0; istub<reader.vr_stubRefs->at(iroad).at(isuperstrip).size(); ++istub)
                        stubRefsAllLayers.push_back(reader.vr_stubRefs->at(iroad).at(isuperstrip).at(istub));
                assert(nstubs == stubRefsAllLayers.size());

                if (verbose_>2) {
                    std::cout << Debug() << "... ... road: " << iroad << " tower: " << tower << " # stubs: " << nstubs << std::endl;
                    std::cout << Debug() << "... ... road: " << iroad << " stubRefs: ";
                    std::copy(stubRefsAllLayers.begin(), stubRefsAllLayers.end(), std::ostream_iterator<unsigned>(std::cout, " "));
                    std::cout << std::endl;
                }

                // _____________________________________________________________
                // Loop over the stubs
                std::vector<TTHit> hits;
                for (unsigned istub=0; istub<nstubs; ++istub) {
                    const unsigned& ref = stubRefsAllLayers.at(istub);
                    hits.emplace_back(TTHit{                // using POD type constructor
                        ref,
                        reader.vb_r->at(ref),
                        reader.vb_phi->at(ref),
                        reader.vb_z->at(ref),
                        0.,  // FIXME: add errors
                        0.,
                        0.
                    });
                }  // loop over the stubs

                // _____________________________________________________________
                // Fit

                fitstatus = fitterRetina_->fit(tower, ievt, iroad, hits, tracks);

                if (verbose_>2)  std::cout << Debug() << "... ... road: " << iroad << " # tracks: " << tracks.size() << " status: " << fitstatus << std::endl;

                /// Debug
                if (verbose_>3 && tracks.size()>0) {
                    std::cout << " ----------------------------------------------------------------" << std::endl;
                    std::cout << " Generated particles:" << std::endl;

                    for (unsigned ipart=0; ipart<reader.vp_pt->size(); ++ipart) {
                        float pt      = reader.vp_pt     ->at(ipart);
                        float phi     = reader.vp_phi    ->at(ipart);
                        float eta     = reader.vp_eta    ->at(ipart);
                        float vx      = reader.vp_vx     ->at(ipart);
                        float vy      = reader.vp_vy     ->at(ipart);
                        float vz      = reader.vp_vz     ->at(ipart);
                        int   charge  = reader.vp_charge ->at(ipart);

                        // curvature and helix radius
                        float R       = pt/(0.003*3.8);

                        // helix center
                        float x0      = vx - charge*R*std::sin(phi);
                        float y0      = vy + charge*R*std::cos(phi);

                        // transverse and longitudinal impact parameters
                        float d0      = charge*(std::sqrt(x0*x0+y0*y0)-R);
                        float theta   = 2.*std::atan(std::exp(-eta));
                        float z0      = vz - 2.*R/std::tan(theta)*
                                        std::asin(0.5/R*std::sqrt((vx*vx+vy*vy-d0*d0)/(1.+d0/R)));

                        std::cout << "  " << ipart << "  -  " << pt << " " << phi << " " << eta << " " << z0 << std::endl;
                    }

                    std::cout << " Fitted tracks:" << std::endl;
                    for (unsigned itrk=0; itrk<tracks.size(); ++itrk) {
                        const TTTrack2& track = tracks.at(itrk);
                        const std::vector<unsigned>& stubRefs = track.stubRefs();

                        std::cout << "  " << itrk << "  -  " << track.pt() << " " << track.phi0() << " " << track.eta() << " " << track.z0() << " " << stubRefs.size() << std::endl;
                    }
                }

            }  // loop over the roads
            break;


        case 1:
        default:
            // _________________________________________________________________
            // Track fitters taking fit combinations

            // Loop over the roads
            for (unsigned iroad=0; iroad<nroads; ++iroad) {

                const unsigned tower = reader.vr_tower->at(iroad);
                const unsigned nstubs = reader.vr_nstubs->at(iroad);

                const std::vector<std::vector<unsigned> >& combinations = arrangeCombinations(reader.vr_stubRefs->at(iroad)); // get combinations of stubRefs

                for (unsigned icomb=0; icomb<combinations.size(); ++icomb)
                    assert(combinations.at(icomb).size() == reader.vr_stubRefs->at(iroad).size());

                if (verbose_>2) {
                    std::cout << Debug() << "... ... road: " << iroad << " tower: " << tower << " # stubs: " << nstubs << " # combinations: " << combinations.size() << std::endl;
                }

                // _____________________________________________________________
                // Loop over the combinations

                for (unsigned icomb=0; icomb<combinations.size(); ++icomb) {
                    const std::vector<unsigned>& stubRefs = combinations.at(icomb);

                    if (verbose_>2) {
                        std::cout << Debug() << "... ... ... comb: " << icomb << " stubRefs: ";
                        std::copy(stubRefs.begin(), stubRefs.end(), std::ostream_iterator<unsigned>(std::cout, " "));
                        std::cout << std::endl;
                    }

                    // Loop over the stubs
                    std::vector<TTHit> hits;
                    for (unsigned istub=0; istub<stubRefs.size(); ++istub) {
                        const unsigned& ref = stubRefs.at(istub);
                        if (ref == 999999)  // empty
                            continue;

                        hits.emplace_back(TTHit{                // using POD type constructor
                            ref,
                            reader.vb_r->at(ref),
                            reader.vb_phi->at(ref),
                            reader.vb_z->at(ref),
                            0.,  // FIXME: add errors
                            0.,
                            0.
                        });
                    }

                    // _________________________________________________________
                    // Fit
                    TTTrack2 atrack;
                    atrack.setRoadRef(iroad);
                    atrack.setTower(tower);
                    atrack.setStubRefs(stubRefs);

                    if (po_.mode == 1)
                        fitstatus = fitterDas_->fit(hits, atrack);
                    else
                        fitstatus = fitterLin_->fit(hits, atrack);
                    tracks.push_back(atrack);

                    if (verbose_>2)  std::cout << Debug() << "... ... ... track: " << icomb << " status: " << fitstatus << std::endl;
                }
            }  // loop over the roads

            break;
        }

        if (verbose_>3) {
            for (unsigned itrack=0; itrack!=tracks.size(); ++itrack) {
                std::cout << "... ... track: " << itrack << " " << tracks.at(itrack) << std::endl;
            }
        }

        // _____________________________________________________________________
        // Remove fails and duplicates

        // FIXME: implement this

        if (! tracks.empty())
            ++nKept;

        writer.fill(tracks);
        ++nRead;
    }

    if (verbose_)  std::cout << Info() << Form("Read: %7ld, triggered: %7ld", nRead, nKept) << std::endl;

    long long nentries = writer.writeTree();
    assert(nentries == nRead);

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
