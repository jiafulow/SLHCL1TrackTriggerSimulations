#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternMatcher.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"

static const unsigned MAX_NLAYERS = pattern_type().size();  // ought to be 8


// _____________________________________________________________________________
// Do pattern recognition
int PatternMatcher::makeRoads_fas(TString src, TString bank, TString out) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and matching patterns." << std::endl;

    // _________________________________________________________________________
    // For reading pattern bank
    PatternBankReader pbreader(verbose_);
    if (pbreader.init(bank)) {
        std::cout << Error() << "Failed to initialize PatternBankReader." << std::endl;
        return 1;
    }

    long long npatterns = pbreader.getPatterns();
    if (npatterns > maxPatterns_)
        npatterns = maxPatterns_;

    id_type fakeSuperstrip;
    if (po.mode == 2) {  // luciano superstrip
        fakeSuperstrip = arbiter_ -> superstrip_luciano(27, 0, 0, po.unitPhi, po.unitEta);
    } else {
        fakeSuperstrip = arbiter_ -> superstrip(27, 0, 0, 0, 0);
    }

    id_type nvalues = fakeSuperstrip + 1;
    if (nvalues & 1) nvalues += 1;  // if odd, make it even

    if (verbose_)  std::cout << Info() << "Loading " << npatterns << " patterns, assuming " << nvalues << " possible superstripIds. " << std::endl;
    assert(npatterns > 0 && nvalues > 0);

    // Allocate memory
    inputPatterns_fas_.init(MAX_NLAYERS, npatterns, nvalues);
    if (verbose_>2)  std::cout << Debug() << "Initialized inputPatterns_fas_" << std::endl;

#ifdef USE_LEAN_COUNTER
    fas::lean_counter3 patternFinder(0,0);
#else
    fas::lean_merger3 patternFinder(0,0);
#endif
    patternFinder.init(nLayers_, nLayers_ - po.nMisses, inputPatterns_fas_.num_keys());

    // _________________________________________________________________________
    // Loop over all patterns

    for (long long ipatt=0; ipatt<npatterns; ++ipatt) {
        pbreader.getPattern(ipatt);
        if (pbreader.pb_frequency < minFrequency_)
            break;

        assert(pbreader.pb_superstripIds->size() == MAX_NLAYERS);

        // Here we insert all superstrips including the virtual ones (calo, muon)
        // When doing the matching, the virtual ones are not used
        id_type * addr_ptr = std::addressof(pbreader.pb_superstripIds->at(0));
        inputPatterns_fas_.insert(addr_ptr, addr_ptr + MAX_NLAYERS);

        if (verbose_>3) {
            for (unsigned i=0; i<MAX_NLAYERS; ++i) {
                std::cout << Debug() << "... patt: " << i << "  ";
                std::copy(pbreader.pb_superstripIds->begin(), pbreader.pb_superstripIds->end(), std::ostream_iterator<id_type>(std::cout, " "));
                std::cout << " freq: " << (unsigned) pbreader.pb_frequency << std::endl;
            }
        }
    }

    if ((size_t) npatterns * MAX_NLAYERS == inputPatterns_fas_.size()) {
        if (verbose_)  std::cout << Info() << "Successfully loaded " << npatterns << " patterns." << std::endl;
    } else {
        std::cout << Error() << "Failed to load all patterns: " << inputPatterns_fas_.size() << " vs " << npatterns * MAX_NLAYERS << std::endl;
        return 1;
    }

    // Generate the reverse look up table
    inputPatterns_fas_.reverse();

    // _________________________________________________________________________
    // Get the trigger tower maps
    //pbreader.getTriggerTowerMaps(triggerTowerMap_, triggerTowerReverseMap_);
    //
    //if (triggerTowerMap_.empty() || triggerTowerReverseMap_.empty()) {
    //    std::cout << Error() << "Failed to load trigger tower maps" << std::endl;
    //    return 1;
    //}

    // _________________________________________________________________________
    // For reading events
    TTStubReader reader(verbose_);
    if (reader.init(src)) {
        std::cout << Error() << "Failed to initialize TTStubReader." << std::endl;
        return 1;

    } else {
        TChain* tchain = reader.getChain();
        tchain->SetBranchStatus("*"                 , 0);
        tchain->SetBranchStatus("genParts_pt"       , 1);
        tchain->SetBranchStatus("genParts_eta"      , 1);
        tchain->SetBranchStatus("genParts_phi"      , 1);
        tchain->SetBranchStatus("genParts_vx"       , 1);
        tchain->SetBranchStatus("genParts_vy"       , 1);
        tchain->SetBranchStatus("genParts_vz"       , 1);
        tchain->SetBranchStatus("genParts_charge"   , 1);
        tchain->SetBranchStatus("TTStubs_x"         , 1);
        tchain->SetBranchStatus("TTStubs_y"         , 1);
        tchain->SetBranchStatus("TTStubs_z"         , 1);
        tchain->SetBranchStatus("TTStubs_r"         , 1);
        tchain->SetBranchStatus("TTStubs_eta"       , 1);
        tchain->SetBranchStatus("TTStubs_phi"       , 1);
        tchain->SetBranchStatus("TTStubs_coordx"    , 1);
        tchain->SetBranchStatus("TTStubs_coordy"    , 1);
        tchain->SetBranchStatus("TTStubs_roughPt"   , 1);
        tchain->SetBranchStatus("TTStubs_trigBend"  , 1);
      //tchain->SetBranchStatus("TTStubs_clusWidth" , 1);
        tchain->SetBranchStatus("TTStubs_modId"     , 1);
        tchain->SetBranchStatus("TTStubs_tpId"      , 1);
    }

    // For writing
    TTRoadWriter writer(verbose_);
    if (writer.init(reader.getChain(), out, prefixRoad_, suffix_)) {
        std::cout << Error() << "Failed to initialize TTRoadWriter." << std::endl;
        return 1;
    }


    // _________________________________________________________________________
    // Loop over all events

    // Containers
    std::vector<unsigned> stubRefs;           // a serialized list of stubs in a road
    std::vector<unsigned> stubSuperstripIds;  // the superstrip ids of the stubs

    std::map<id_type, std::vector<unsigned> > superstripToStubsMap;  // key: superstrip addr; values: vector of stub indices in the superstrip
    //std::vector<bool> superstripBooleans;     // index: superstrip addr; true: found
    //superstripBooleans.resize(nvalues);

    std::vector<TTRoad> roads;
    roads.reserve(300);

    // Bookkeepers
    int nPassed = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%5000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, triggering: %7i", ievt, nKept, nPassed) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            writer.fill(std::vector<TTRoad>());
            ++nKept;
            continue;
        }

        if (nstubs > 100000) {
            std::cout << Error() << "Way too many stubs: " << nstubs << std::endl;
            return 1;
        }

        // _____________________________________________________________________
        // Start pattern recognition
        superstripToStubsMap.clear();
        //std::fill(superstripBooleans.begin(), superstripBooleans.end(), false);

        // Loop over reconstructed stubs
        id_type ssId, moduleId, lay, lad, mod, col, row;  // declare the usual suspects
        float stub_r, stub_eta, stub_phi, stub_z, stub_trigBend;
        for (unsigned l=0; l<nstubs; ++l) {

            // Break moduleId into lay, lad, mod
            moduleId = reader.vb_modId->at(l);
            lay = decodeLayer(moduleId);
            lad = decodeLadder(moduleId);
            mod = decodeModule(moduleId);

            // col <-- coordy, row <-- coordx
            // use half-strip unit
            col = halfStripRound(reader.vb_coordy->at(l));
            row = halfStripRound(reader.vb_coordx->at(l));

            // global r, eta, phi, z
            stub_r = reader.vb_r->at(l);
            stub_eta = reader.vb_eta->at(l);
            stub_phi = reader.vb_phi->at(l);
            stub_z = reader.vb_z->at(l);
            stub_trigBend = reader.vb_trigBend->at(l);

            //// Skip if moduleId not in any trigger tower
            //if (po.requireTriggerTower && triggerTowerReverseMap_.find(moduleId) == triggerTowerReverseMap_.end()) {
            //    if (verbose_>2)  std::cout << Debug() << "... ... skip moduleId: " << moduleId << " not in any trigger tower." << std::endl;
            //    continue;
            //}

            // Find superstrip address
            if (po.mode == 2) {  // luciano superstrip
                ssId = arbiter_ -> superstrip_luciano(lay, stub_phi, stub_eta, po.unitPhi, po.unitEta);
            } else {
                ssId = arbiter_ -> superstrip(lay, lad, mod, col, row);
            }

            // Create a link from superstrip to stub index
            superstripToStubsMap[ssId].push_back(l);

            // Make a tick
            //superstripBooleans.at(ssId) = true;

            if (verbose_>2) {
                std::cout << Debug() << "... ... stub: " << l << " moduleId: " << moduleId << " col: " << col << " row: " << row << " r: " << stub_r << " eta: " << stub_eta << " phi: " << stub_phi << std::endl;
                std::cout << Debug() << "... ... stub: " << l << " ssId: " << ssId << std::endl;
            }
        }

        // Always trigger fake superstrip
        superstripToStubsMap[fakeSuperstrip] = std::vector<unsigned>();
        //superstripBooleans.at(fakeSuperstrip) = true;

        // Limit the max number of stubs that can be read out in a superstrip
        for (std::map<id_type, std::vector<unsigned> >::iterator it = superstripToStubsMap.begin();
             it != superstripToStubsMap.end(); ++it) {
            if ((int) it->second.size() > maxStubs_)
                it->second.resize(maxStubs_);
        }


        // _____________________________________________________________________
        // Create roads
        roads.clear();

        stubRefs.clear();
        stubSuperstripIds.clear();
        count_type nsuperstrips = 0;

#ifdef USE_LEAN_COUNTER
        // Perform reverse lookup
        for (std::map<id_type, std::vector<unsigned> >::const_iterator it = superstripToStubsMap.begin();
             it != superstripToStubsMap.end(); ++it) {
            patternFinder.insert(it->first, &inputPatterns_fas_);
        }
        const fas::lean_counter3::return_key_range& ret_keys = patternFinder.count();

        // Collect stubs
        for (std::map<id_type, std::vector<unsigned> >::const_iterator it = superstripToStubsMap.begin();
             it != superstripToStubsMap.end(); ++it) {
            patternFinder.insert_again(it->first, &inputPatterns_fas_);
        }
        const fas::lean_counter3::return_value_range& ret_values = patternFinder.count_again();

        fas::lean_counter3::value_type * itss = ret_values.begin;
        std::map<id_type, std::vector<unsigned> >::const_iterator found;
        std::vector<unsigned>::const_iterator itfound;

        for (fas::lean_counter3::key_type * it = ret_keys.begin; it != ret_keys.end; ++it) {
            stubRefs.clear();
            stubSuperstripIds.clear();
            nsuperstrips = 0;

            for (unsigned i=0; i<nLayers_; ++i, ++itss) {
                const id_type& ssId = *itss;
                if (ssId == fas::bad_4byte)  continue;

                found = superstripToStubsMap.find(ssId);
                if (found != superstripToStubsMap.end()) {
                    if (!found->second.empty())
                        ++nsuperstrips;
                    for (itfound = found->second.begin(); itfound != found->second.end(); ++itfound) {
                        stubRefs.push_back(*itfound);
                        stubSuperstripIds.push_back(ssId);
                    }
                } else {
                    // ??
                }
            }

            assert(stubSuperstripIds.size() == stubRefs.size());
            TTRoad road(*it, 99, nsuperstrips, stubSuperstripIds, stubRefs);
            roads.push_back(road);

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << roads.size() - 1 << " " << roads.back() << std::endl;

            if ((int) roads.size() >= maxRoads_)
                break;
        }

#else
        // Perform reverse lookup
        for (std::map<id_type, std::vector<unsigned> >::const_iterator it = superstripToStubsMap.begin();
             it != superstripToStubsMap.end(); ++it) {
            patternFinder.insert(it->first, &inputPatterns_fas_);
        }
        const fas::lean_merger3::return_type& ret_pairs = patternFinder.merge();

        // Collect stubs
        fas::lean_merger3::pair_type * it = ret_pairs.begin;
        std::map<id_type, std::vector<unsigned> >::const_iterator found;
        std::vector<unsigned>::const_iterator itfound;

        while (it != ret_pairs.end) {
            stubRefs.clear();
            stubSuperstripIds.clear();
            nsuperstrips = 0;

            const unsigned ipatt = it->key;
            for (; (it != ret_pairs.end) && (it->key == ipatt); ++it) {
                const id_type& ssId = it->value;

                found = superstripToStubsMap.find(ssId);
                if (found != superstripToStubsMap.end()) {
                    if (!found->second.empty())
                        ++nsuperstrips;
                    for (itfound = found->second.begin(); itfound != found->second.end(); ++itfound) {
                        stubRefs.push_back(*itfound);
                        stubSuperstripIds.push_back(ssId);
                    }
                } else {
                    // ??
                }
            }

            assert(superstripIds_.size() == stubRefs_.size());
            TTRoad road(ipatt, 99, nsuperstrips, stubSuperstripIds, stubRefs);
            roads.push_back(road);

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << roads.size() - 1 << " " << roads.back() << std::endl;

            if ((int) roads.size() >= maxRoads_)
                break;
        }
#endif

        if (! roads.empty())
            ++nPassed;

        writer.fill(roads);
        ++nKept;
    }

    if (verbose_)  std::cout << Info() << "Processed " << nEvents_ << " events, kept " << nKept << ", triggered on " << nPassed << std::endl;

    long long nentries = writer.writeTree();
    assert(nentries == nKept);

    return 0;
}


// _____________________________________________________________________________
// Private functions
// none


// _____________________________________________________________________________
// Main driver
int PatternMatcher::run(TString src, TString bank, TString out) {
    int exitcode = 0;
    Timing(1);

    bool use_fas = true;
    if (use_fas) {
        exitcode = makeRoads_fas(src, bank, out);
        if (exitcode)  return exitcode;
        Timing();

    } else {
        if (verbose_)  std::cout << Info() << "Switched off fas" << std::endl;

        exitcode = makeRoads_vector(src, bank, out);
        if (exitcode)  return exitcode;
        Timing();
    }

    return exitcode;
}
