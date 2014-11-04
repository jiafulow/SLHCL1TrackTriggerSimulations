#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternGenerator.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/CSVFileReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"

static const unsigned MIN_NGOODSTUBS = 3;
static const unsigned MAX_NGOODSTUBS = 8;
static const unsigned MAX_NTOWERS = 6 * 8;
static const unsigned MAX_NMODULES = 15428;
static const unsigned MAX_FREQUENCY = 0xffff;  // 65535


// _____________________________________________________________________________
// Parse the trigger tower csv file
int PatternGenerator::readTriggerTowerFile(TString src) {
    CSVFileReader csvreader(verbose_);
    if (csvreader.getTriggerTowerMap(src, triggerTowerMap_)) {
        std::cout << Error() << "Failed to parse the trigger tower csv file." << std::endl;
        return 1;
    }

    assert(triggerTowerMap_.size() == MAX_NTOWERS);

    //for (auto it: triggerTowerMap_)
    //    std::cout << "Tower " << it.first << " has " << it.second.size() << " modules." << std::endl;

    // Reverse trigger tower map
    triggerTowerReverseMap_.clear();
    for (auto it: po.triggerTowers) { // loop over input trigger towers
        const std::vector<unsigned>& moduleIds = triggerTowerMap_[it];
        for (auto it2: moduleIds) {   // loop over the moduleIds in the tower
            triggerTowerReverseMap_[it2].push_back(it);
        }
    }

    //for (auto it: triggerTowerReverseMap_)
    //    std::cout << "Module " << it.first << " is in " << it.second.size() << " towers." << std::endl;
    return 0;
}


// _____________________________________________________________________________
// Parse the module coordinate csv file
int PatternGenerator::readModuleCoordinateFile(TString src) {
    CSVFileReader csvreader(verbose_);
    if (csvreader.getModuleCoordinateMap(src, moduleCoordinateMap_)) {
        std::cout << Error() << "Failed to parse the module coordinate csv file." << std::endl;
        return 1;
    }

    assert(moduleCoordinateMap_.size() == MAX_NMODULES);

    //for (auto it: moduleCoordinateMap_)
    //    std::cout << "Module " << it.first << " coordinate: " << it.second.rho << "," << it.second.eta << "," << it.second.phi << std::endl;
    return 0;
}


// _____________________________________________________________________________
// Make the patterns
int PatternGenerator::makePatterns_fas(TString src) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and generating patterns." << std::endl;

    // _________________________________________________________________________
    // For reading events
    TTStubReader reader(verbose_);
    if (reader.init(src, false)) {
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
      //tchain->SetBranchStatus("TTStubs_roughPt"   , 1);
      //tchain->SetBranchStatus("TTStubs_trigBend"  , 1);
        tchain->SetBranchStatus("TTStubs_modId"     , 1);
      //tchain->SetBranchStatus("TTStubs_trkId"     , 1);
        tchain->SetBranchStatus("genParts_pt"       , 1);
      //tchain->SetBranchStatus("genParts_eta"      , 1);
      //tchain->SetBranchStatus("genParts_phi"      , 1);
      //tchain->SetBranchStatus("genParts_vx"       , 1);
      //tchain->SetBranchStatus("genParts_vy"       , 1);
      //tchain->SetBranchStatus("genParts_vz"       , 1);
      //tchain->SetBranchStatus("genParts_charge"   , 1);
    }

    id_type caloSuperstrip, muonSuperstrip, fakeSuperstrip;
    if (po.mode == 2) {  // luciano superstrip
        caloSuperstrip = arbiter_ -> superstrip_luciano(25, 0, 0, po.unitPhi, po.unitZ);
        muonSuperstrip = arbiter_ -> superstrip_luciano(26, 0, 0, po.unitPhi, po.unitZ);
        fakeSuperstrip = arbiter_ -> superstrip_luciano(27, 0, 0, po.unitPhi, po.unitZ);
    } else {
        caloSuperstrip = arbiter_ -> superstrip(25, 0, 0, 0, 0);
        muonSuperstrip = arbiter_ -> superstrip(26, 0, 0, 0, 0);
        fakeSuperstrip = arbiter_ -> superstrip(27, 0, 0, 0, 0);
    }

    // Allocate memory
    allPatterns_fas_.init(0);  // automatic memory allocation
    if (verbose_>2)  std::cout << Debug() << "Initialized allPatterns_fas_." << std::endl;


    // _________________________________________________________________________
    // Loop over all events

    // Containers
    std::vector<id_type> stubLayers;
    pattern_type patt;
    pattern_type pattEmpty;  // for sanity check

    std::map<unsigned, unsigned> towerCountMap;

    // Bookkeepers
    float bankSize_f = 0., bankOldSize_f = 0., nKept_f = 0., nKeptOld_f = 0.;
    int nRead = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%100000==0) {
            // Coverage info
            bankSize_f = allPatterns_fas_.size();
            nKept_f = nKept;
            coverage_ = 1. - (bankSize_f - bankOldSize_f) / (nKept_f - nKeptOld_f);
            std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, # patterns: %7.0f, coverage: %7.5f", ievt, nKept, bankSize_f, coverage_) << std::endl;

            bankOldSize_f = bankSize_f;
            nKeptOld_f = nKept_f;
        }

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (nstubs < MIN_NGOODSTUBS) {  // skip if not enough stubs
            ++nRead;
            continue;
        }

        if (nstubs > MAX_NGOODSTUBS) {
            std::cout << Error() << "Too many stubs: " << nstubs << std::endl;
            return 1;
        }

        float simPt = reader.vp_pt->front();
        if (simPt < po.minPt || po.maxPt < simPt) {
            ++nRead;
            continue;
        }

        // _____________________________________________________________________
        // Start generating patterns
        bool keep = true;

        stubLayers.clear();
        patt.fill(0);
        patt.at(6) = caloSuperstrip;  // dummy for now
        patt.at(7) = muonSuperstrip;  // dummy for now

        // Quick loop over reconstructed stubs
        id_type ssId, moduleId, lay, lad, mod, col, row;  // declare the usual suspects
        float stub_r, stub_phi, stub_z;
        for (unsigned l=0; (l<nstubs) && keep; ++l) {
            moduleId = reader.vb_modId->at(l);

            // Skip if moduleId not in any trigger tower
            if (po.requireTriggerTower && triggerTowerReverseMap_.find(moduleId) == triggerTowerReverseMap_.end()) {
                if (verbose_>2)  std::cout << Debug() << "... ... skip moduleId: " << moduleId << " not in any trigger tower." << std::endl;
                continue;
            }

            lay = decodeLayer(moduleId);
            const unsigned& count = std::count(stubLayers.begin(), stubLayers.end(), lay);
            if (count != 0) {
                std::cout << Error() << "There should be only one stub in any layer" << std::endl;
                return 1;
            }
            stubLayers.push_back(lay);
        }

        // Decide how to lay out the pattern
        //const std::vector<unsigned>& indices = stitcher_ -> stitch(stubLayers);
        const std::vector<unsigned>& indices = stitcher_ -> stitch_layermap(stubLayers);

        if (indices.empty())
            keep = false;

        if (verbose_>2) {
            std::cout << Debug() << "... evt: " << ievt << " moduleIds: ";
            std::copy(stubLayers.begin(), stubLayers.end(), std::ostream_iterator<unsigned>(std::cout, " "));
            std::cout << "  indices: ";
            std::copy(indices.begin(), indices.end(), std::ostream_iterator<unsigned>(std::cout, " "));
            std::cout << std::endl;
        }
        assert(!keep || indices.size() == nLayers_);

        // _____________________________________________________________________
        // Encode superstrip id
        towerCountMap.clear();  // count trigger towers for all superstrips

        // Loop over reconstructed stubs
        for (unsigned k=0, l=0; (k<nLayers_) && keep; ++k) {
            // Fake superstrip
            if (indices.at(k) == 999999) {
                // Arbitrarily put fake superstrip in layer 27
                patt.at(k) = fakeSuperstrip;
                continue;
            }

            // Real superstrip (tracker only)
            assert(k < nstubs && indices.at(k) < nstubs);
            l = indices.at(k);

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

            // Find superstrip address
            if (po.mode == 2) {  // luciano superstrip
                ssId = arbiter_ -> superstrip_luciano(lay, stub_phi, stub_z, po.unitPhi, po.unitZ);
            } else {
                ssId = arbiter_ -> superstrip(lay, lad, mod, col, row);
            }
            patt.at(k) = ssId;

            // Find associated trigger towers
            if (po.requireTriggerTower) {
                const std::vector<unsigned>& towerIds = triggerTowerReverseMap_.at(moduleId);
                for (unsigned i=0; i<towerIds.size(); ++i) {
                    ++towerCountMap[towerIds.at(i)];
                }
                ++towerCountMap[999999];  // total count
            }

            if (verbose_>2)  std::cout << Debug() << "... ... stub: " << l << " moduleId: " << moduleId << " col: " << col << " row: " << row << " ssId: " << ssId << std::endl;
        }

        if (keep && po.requireTriggerTower) {
            // Drop patterns that are not within any trigger tower
            std::map<unsigned, unsigned>::const_iterator ittower;
            const unsigned total = towerCountMap.at(999999);
            towerCountMap.erase(999999);

            keep = false;
            for (ittower = towerCountMap.begin(); ittower != towerCountMap.end(); ++ittower) {
                if (ittower->second == total) {
                    keep = true;
                    break;
                }
            }
        }

        assert (!keep || patt != pattEmpty);

        // _____________________________________________________________________
        // Insert pattern
        if (keep) {
            allPatterns_fas_.insert(patt.data(), patt.data() + patt.size());
            //allPatterns_fas_.insert_nosort(patt.data(), patt.data() + patt.size());
            ++nKept;

            if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " patt: " << patt << std::endl;
        }
        ++nRead;
    }
    if (verbose_)  std::cout << Info() << Form("Read: %7i, kept: %7i, # patterns: %7u, coverage: %7.5f", nRead, nKept, allPatterns_fas_.size(), coverage_) << std::endl;

    // Keep this number to calculate sorted coverage
    coverage_count_ = nKept;

    // _________________________________________________________________________
    // Sort by frequency

    allPatterns_fas_.sort();
    //assert(allPatterns_fas_.at(0).count <= MAX_FREQUENCY);

    if (verbose_>2) {
        for (unsigned i=0; i<allPatterns_fas_.size(); ++i) {
            const fas::lean_table3::return_type& ret = allPatterns_fas_.at(i);
            std::cout << Debug() << "... patt: " << i << "  ";
            std::copy(ret.begin, ret.end, std::ostream_iterator<id_type>(std::cout, " "));
            std::cout << " freq: " << ret.count << std::endl;
        }
    }

    unsigned highest_freq = allPatterns_fas_.size() ? allPatterns_fas_.at(0).count : 0;
    if (verbose_)  std::cout << Info() << "Generated " << allPatterns_fas_.size() << " patterns, highest freq: " << highest_freq << std::endl;

    return 0;
}


// _____________________________________________________________________________
// Output patterns into a TTree
int PatternGenerator::writePatterns_fas(TString out) {

    // _________________________________________________________________________
    // For writing
    PatternBankWriter writer(verbose_);
    if (writer.init(out)) {
        std::cout << Error() << "Failed to initialize TTRoad writer." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // Save trigger tower maps
    writer.pb_ttmap = &triggerTowerMap_;
    writer.pb_ttrmap = &triggerTowerReverseMap_;
    writer.fillTriggerTower();

    // _________________________________________________________________________
    // Save pattern bank
    const long long nentries = allPatterns_fas_.size();

    float integralFreq_f = 0;
    unsigned integralFreq = 0;
    count_type freq = MAX_FREQUENCY, oldFreq = MAX_FREQUENCY;

    for (long long ievt=0; ievt<nentries; ++ievt) {
        if (verbose_>1 && ievt%10000==0) {
            integralFreq_f = integralFreq;
            float coverage = integralFreq_f / coverage_count_ * coverage_;
            std::cout << Debug() << Form("... Writing event: %7lld, sorted coverage: %7.5f", ievt, coverage) << std::endl;
        }

        const fas::lean_table3::return_type& ret = allPatterns_fas_.at(ievt);
        freq = ret.count;

        // Make sure patterns are indeed sorted by frequency
        assert(oldFreq >= freq);
        oldFreq = freq;
        integralFreq += freq;

        if (freq < minFrequency_)  // cut off
            break;

        *(writer.pb_frequency) = freq;
        writer.pb_superstripIds->clear();
        for (id_type * it = ret.begin; it != ret.end; ++it) {
            writer.pb_superstripIds->push_back(*it);
        }
        writer.fillPatternBank();
    }

    long long nentries2 = writer.write();
    assert(nentries == nentries2);

    return 0;
}


// _____________________________________________________________________________
// Private functions
// none


// _____________________________________________________________________________
// Main driver
int PatternGenerator::run(TString src, TString datadir, TString out) {
    int exitcode = 0;
    Timing(1);

    exitcode = readTriggerTowerFile(datadir + "trigger_sector_map.csv");
    if (exitcode)  return exitcode;
    Timing();

    //exitcode = readModuleCoordinateFile(datadir + "module_coordinates.csv");
    //if (exitcode)  return exitcode;
    //Timing();

    bool use_fas = true;
    if (use_fas) {
        exitcode = makePatterns_fas(src);
        if (exitcode)  return exitcode;
        Timing();

        exitcode = writePatterns_fas(out);
        if (exitcode)  return exitcode;
        Timing();

    } else {
        if (verbose_)  std::cout << Info() << "Switched off fas" << std::endl;

        exitcode = makePatterns_map(src);
        if (exitcode)  return exitcode;
        Timing();

        exitcode = writePatterns_map(out);
        if (exitcode)  return exitcode;
        Timing();
    }

    return exitcode;
}
