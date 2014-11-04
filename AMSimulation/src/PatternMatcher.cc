#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternMatcher.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"
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
        fakeSuperstrip = arbiter_ -> superstrip_luciano(27, 0, 0, po.unitPhi, po.unitZ);
    } else {
        fakeSuperstrip = arbiter_ -> superstrip(27, 0, 0, 0, 0);
    }

    id_type nsuperstrips = fakeSuperstrip + 1;
    if (nsuperstrips & 1) nsuperstrips += 1;  // if odd, make it even

    if (verbose_)  std::cout << Info() << "Loading " << npatterns << " patterns, assuming " << nsuperstrips << " superstrips. " << std::endl;
    assert(npatterns > 0 && nsuperstrips > 0);

    // Allocate memory
    inputPatterns_fas_.init(MAX_NLAYERS, npatterns, nsuperstrips);
    if (verbose_>2)  std::cout << Debug() << "Initialized inputPatterns_fas_" << std::endl;

#ifdef USE_LEAN_COUNTER
    fas::lean_counter3 patternFinder(0,0);
#else
    fas::lean_merger3 patternFinder(0,0);
#endif
    patternFinder.init(nLayers_, nLayers_ - po.nMisses, inputPatterns_fas_.num_keys());

    // _________________________________________________________________________
    // Loop over all patterns

    for (long long ievt=0; ievt<npatterns; ++ievt) {
        pbreader.getPattern(ievt);
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
      //tchain->SetBranchStatus("TTStubs_x"         , 1);
      //tchain->SetBranchStatus("TTStubs_y"         , 1);
        tchain->SetBranchStatus("TTStubs_z"         , 1);
        tchain->SetBranchStatus("TTStubs_r"         , 1);
      //tchain->SetBranchStatus("TTStubs_eta"       , 1);
        tchain->SetBranchStatus("TTStubs_phi"       , 1);
        tchain->SetBranchStatus("TTStubs_coordx"    , 1);
        tchain->SetBranchStatus("TTStubs_coordy"    , 1);
        tchain->SetBranchStatus("TTStubs_roughPt"   , 1);
        tchain->SetBranchStatus("TTStubs_trigBend"  , 1);
        tchain->SetBranchStatus("TTStubs_modId"     , 1);
        tchain->SetBranchStatus("TTStubs_trkId"     , 1);
        tchain->SetBranchStatus("genParts_pt"       , 1);
        tchain->SetBranchStatus("genParts_eta"      , 1);
        tchain->SetBranchStatus("genParts_phi"      , 1);
        tchain->SetBranchStatus("genParts_vx"       , 1);
        tchain->SetBranchStatus("genParts_vy"       , 1);
        tchain->SetBranchStatus("genParts_vz"       , 1);
        tchain->SetBranchStatus("genParts_charge"   , 1);
    }

    // For writing
    TTRoadWriter writer(verbose_);
    if (writer.init(out, prefixRoad_, suffix_)) {
        std::cout << Error() << "Failed to initialize TTRoadWriter." << std::endl;
        return 1;
    }


    // _________________________________________________________________________
    // Loop over all events

    // Containers
    std::vector<std::vector<TTHit> > hitses;  // just like hits, but before serialized
    std::vector<TTHit> hits;                  // after serialized

    std::map<id_type, std::vector<TTHit> > superstripHitsMap;   // key: superstrip addr; values: vector of hits
    //std::vector<bool> superstripBooleans;                       // index: superstrip addr; true: found
    //superstripBooleans.resize(nsuperstrips);

    std::vector<TTRoad> roads;
    roads.reserve(300);
    std::vector<GenParticle> genParts;
    genParts.reserve(50);

    // Bookkeepers
    int nPassed = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%5000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, triggering: %7i", ievt, nKept, nPassed) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            writer.fill(std::vector<TTRoad>(), std::vector<GenParticle>());
            ++nKept;
            continue;
        }

        if (nstubs > 100000) {
            std::cout << Error() << "Way too many stubs: " << nstubs << std::endl;
            return 1;
        }

        // _____________________________________________________________________
        // Start pattern recognition
        superstripHitsMap.clear();
        //std::fill(superstripBooleans.begin(), superstripBooleans.end(), false);

        // Loop over reconstructed stubs
        id_type ssId, moduleId, lay, lad, mod, col, row;  // declare the usual suspects
        float stub_r, stub_phi, stub_z, stub_pt;
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

            // global r, phi, z
            stub_r = reader.vb_r->at(l);
            stub_phi = reader.vb_phi->at(l);
            stub_z = reader.vb_z->at(l);
            stub_pt = reader.vb_roughPt->at(l);

            // Skip if moduleId not in any trigger tower
            if (po.requireTriggerTower && triggerTowerReverseMap_.find(moduleId) == triggerTowerReverseMap_.end()) {
                if (verbose_>2)  std::cout << Debug() << "... ... skip moduleId: " << moduleId << " not in any trigger tower." << std::endl;
                continue;
            }

            // Find superstrip address
            if (po.mode == 2) {  // luciano superstrip
                ssId = arbiter_ -> superstrip_luciano(lay, stub_phi, stub_z, po.unitPhi, po.unitZ);
            } else {
                ssId = arbiter_ -> superstrip(lay, lad, mod, col, row);
            }

            // Create a hit
            const int& trkId = reader.vb_trkId->at(l);
            superstripHitsMap[ssId].emplace_back(TTHit{stub_r, stub_phi, stub_z, 0., 0., 0., -1, stub_pt, ssId, trkId});  // POD type constructor

            // Make a tick
            //superstripBooleans.at(ssId) = true;

            if (verbose_>2)  std::cout << Debug() << "... ... stub: " << l << " moduleId: " << moduleId << " col: " << col << " row: " << row << " ssId: " << ssId << std::endl;
        }

        // Always trigger fake superstrip
        superstripHitsMap[fakeSuperstrip] = std::vector<TTHit>();
        //superstripBooleans.at(fakeSuperstrip) = true;


        // _____________________________________________________________________
        // Create roads
        roads.clear();

        hitses.clear();
        hits.clear();

#ifdef USE_LEAN_COUNTER
        // Perform reverse lookup
        for (std::map<id_type, std::vector<TTHit> >::const_iterator it = superstripHitsMap.begin();
             it != superstripHitsMap.end(); ++it) {
            patternFinder.insert(it->first, &inputPatterns_fas_);
        }
        const fas::lean_counter3::return_key_range& ret_keys = patternFinder.count();

        // Collect stubs
        for (std::map<id_type, std::vector<TTHit> >::const_iterator it = superstripHitsMap.begin();
             it != superstripHitsMap.end(); ++it) {
            patternFinder.insert_again(it->first, &inputPatterns_fas_);
        }
        const fas::lean_counter3::return_value_range& ret_values = patternFinder.count_again();

        fas::lean_counter3::value_type * itss = ret_values.begin;
        std::map<id_type, std::vector<TTHit> >::iterator found;

        for (fas::lean_counter3::key_type * it = ret_keys.begin; it != ret_keys.end; ++it) {
            hitses.clear();
            hits.clear();

            for (unsigned i=0; i<MAX_NLAYERS; ++i, ++itss) {
                const id_type& ssId = *itss;
                if (ssId == fas::bad_4byte)  continue;

                found = superstripHitsMap.find(ssId);
                if (found != superstripHitsMap.end()) {
                    hitses.push_back(found -> second);
                    if ((int) hitses.back().size() > maxHits_)
                        hitses.back().resize(maxHits_);
                }
            }

            // Serialize
            for (unsigned i=0; i<hitses.size(); ++i) {
                hits.insert(hits.end(), hitses.at(i).begin(), hitses.at(i).end());
            }
            roads.emplace_back(hitses.size(), *it, hits);

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << roads.size() - 1 << " " << roads.back() << std::endl;

            if ((int) roads.size() >= maxRoads_)
                break;
        }

#else
        // Perform reverse lookup
        for (std::map<id_type, std::vector<TTHit> >::const_iterator it = superstripHitsMap.begin();
             it != superstripHitsMap.end(); ++it) {
            patternFinder.insert(it->first, &inputPatterns_fas_);
        }
        const fas::lean_merger3::return_type& ret_pairs = patternFinder.merge();

        // Collect stubs
        fas::lean_merger3::pair_type * it = ret_pairs.begin;
        std::map<id_type, std::vector<TTHit> >::iterator found;

        while (it != ret_pairs.end) {
            hitses.clear();
            hits.clear();

            const unsigned ipatt = it->key;
            for (; (it != ret_pairs.end) && (it->key == ipatt); ++it) {
                const id_type& ssId = it->value;

                found = superstripHitsMap.find(ssId);
                if (found != superstripHitsMap.end()) {
                    hitses.push_back(found -> second);
                    if ((int) hitses.back().size() > maxHits_)
                        hitses.back().resize(maxHits_);
                }
            }

            // Serialize
            for (unsigned i=0; i<hitses.size(); ++i) {
                hits.insert(hits.end(), hitses.at(i).begin(), hitses.at(i).end());
            }
            roads.emplace_back(hitses.size(), ipatt, hits);

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << roads.size() - 1 << " " << roads.back() << std::endl;

            if ((int) roads.size() >= maxRoads_)
                break;
        }
#endif

        if (! roads.empty())
            ++nPassed;

        // _____________________________________________________________________
        // In addition, keep genParticle info
        genParts.clear();

        const unsigned nparts = reader.vp_pt->size();
        GenParticle part;
        for (unsigned l=0; l<nparts; ++l) {
            part.pt     = reader.vp_pt->at(l);
            part.eta    = reader.vp_eta->at(l);
            part.phi    = reader.vp_phi->at(l);
            part.vx     = reader.vp_vx->at(l);
            part.vy     = reader.vp_vy->at(l);
            part.vz     = reader.vp_vz->at(l);
            part.charge = reader.vp_charge->at(l);
            genParts.push_back(part);
        }

        writer.fill(roads, genParts);
        ++nKept;
    }

    if (verbose_)  std::cout << Info() << "Processed " << nEvents_ << " events, kept " << nKept << ", triggered on " << nPassed << std::endl;

    long long nentries = writer.write();
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
