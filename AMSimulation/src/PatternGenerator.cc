#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternGenerator.h"

static const unsigned MIN_NGOODSTUBS = 3;
static const unsigned MAX_NTOWERS = 6 * 8;
static const unsigned MAX_FREQUENCY = 0xffff;  // 65535

bool sortByFrequency(const std::pair<pattern_type, unsigned>& lhs, const std::pair<pattern_type, unsigned>& rhs) {
    return lhs.second > rhs.second;
}


// _____________________________________________________________________________
// Read and parse the trigger tower csv file
int PatternGenerator::readTriggerTowerFile(TString src) {
    std::vector<unsigned> values;
    if (src.EndsWith(".csv")) {
        if (verbose_)  std::cout << Info() << "Opening " << src << std::endl;

        unsigned i = 0;
        std::string line, issline;
        std::ifstream ifs(src.Data());
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);

            while (std::getline(iss, issline, ',')) {  // split by commas
                if (i != 0)  // skip the first line
                    values.push_back(std::stoi(issline));
            }
            ++i;
        }
    }

    if (values.empty()) {
        std::cout << Error() << "Failed to read the trigger tower file: " << src << std::endl;
        return 1;
    }

    triggerTowerMap_.clear();
    unsigned triggerTowerId = 0;
    for (unsigned i=0; i<values.size(); ++i) {
        if (i == 0)  continue;  // skip the first index
        if (values.at(i-1) <= 6 && values.at(i) <= 8) {  // eta_idx, phi_idx
            triggerTowerId = (values.at(i-1)-1) * 8 + (values.at(i)-1);  // mapped to 0-47
            triggerTowerMap_.insert(std::make_pair(triggerTowerId, std::vector<unsigned>()));
        } else if (values.at(i) > 10000) {
            triggerTowerMap_.at(triggerTowerId).push_back(values.at(i));
        }
    }

    if (triggerTowerMap_.size() != MAX_NTOWERS) {
        std::cout << Error() << "Failed to get all trigger towers from the file: " << src << std::endl;
        return 1;
    }

    //for (auto it: triggerTowerMap_)
    //    std::cout << "Tower " << it.first << " has " << it.second.size() << " modules." << std::endl;

    // Prepare the trigger tower reverse map
    triggerTowerReverseMap_.clear();
    //for (auto it: triggerTowerMap_) { // loop over trigger tower map
    //    for (auto it2: it.second) {   // loop over the moduleIds in the tower
    //        triggerTowerReverseMap_[it2].push_back(it.first);
    //    }
    //}
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
// Read the input ntuples
int PatternGenerator::readFile(TString src) {
    if (src.EndsWith(".root")) {
        if (verbose_)  std::cout << Info() << "Opening " << src << std::endl;
        if (chain_->Add(src) )  // 1 if successful, 0 otherwise
            return 0;

    } else if (src.EndsWith(".txt")) {
        TFileCollection* fc = new TFileCollection("fileinfolist", "", src);
        if (chain_->AddFileInfoList((TCollection*) fc->GetList()) )  // 1 if successful, 0 otherwise
            return 0;
    }

    std::cout << Error() << "Input source should be either a .root file or a .txt file." << std::endl;
    return 1;
}


// _____________________________________________________________________________
// Make the patterns
int PatternGenerator::makePatterns_map() {
    long long nentries = chain_->GetEntries();
    if (nentries <= 0) {
        std::cout << Error() << "Input source has zero entry." << std::endl;
        return 1;
    }
    if (nEvents_ > nentries)
        nEvents_ = nentries;

    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events." << std::endl;

    // For reading
    //std::vector<float> *          vb_x          = 0;
    //std::vector<float> *          vb_y          = 0;
    //std::vector<float> *          vb_z          = 0;
    //std::vector<float> *          vb_r          = 0;
    //std::vector<float> *          vb_phi        = 0;
    std::vector<float> *          vb_coordx     = 0;
    std::vector<float> *          vb_coordy     = 0;
    std::vector<float> *          vb_roughPt    = 0;
    std::vector<unsigned> *       vb_modId      = 0;
    std::vector<unsigned> *       vb_nhits      = 0;
    //std::vector<float> *          vb_simPt      = 0;
    //std::vector<float> *          vb_simEta     = 0;
    //std::vector<float> *          vb_simPhi     = 0;
    //std::vector<int> *            vb_trkId      = 0;

    chain_->SetBranchStatus("*"                 , 0);
    //chain_->SetBranchStatus("TTStubs_x"         , 1);
    //chain_->SetBranchStatus("TTStubs_y"         , 1);
    //chain_->SetBranchStatus("TTStubs_z"         , 1);
    //chain_->SetBranchStatus("TTStubs_r"         , 1);
    //chain_->SetBranchStatus("TTStubs_phi"       , 1);
    chain_->SetBranchStatus("TTStubs_coordx"    , 1);
    chain_->SetBranchStatus("TTStubs_coordy"    , 1);
    chain_->SetBranchStatus("TTStubs_roughPt"   , 1);
    chain_->SetBranchStatus("TTStubs_modId"     , 1);
    chain_->SetBranchStatus("TTStubs_nhits"     , 1);
    //chain_->SetBranchStatus("TTStubs_simPt"     , 1);
    //chain_->SetBranchStatus("TTStubs_simEta"    , 1);
    //chain_->SetBranchStatus("TTStubs_simPhi"    , 1);
    //chain_->SetBranchStatus("TTStubs_trkId"     , 1);

    //chain_->SetBranchAddress("TTStubs_x"        , &(vb_x));
    //chain_->SetBranchAddress("TTStubs_y"        , &(vb_y));
    //chain_->SetBranchAddress("TTStubs_z"        , &(vb_z));
    //chain_->SetBranchAddress("TTStubs_r"        , &(vb_r));
    //chain_->SetBranchAddress("TTStubs_phi"      , &(vb_phi));
    chain_->SetBranchAddress("TTStubs_coordx"   , &(vb_coordx));
    chain_->SetBranchAddress("TTStubs_coordy"   , &(vb_coordy));
    chain_->SetBranchAddress("TTStubs_roughPt"  , &(vb_roughPt));
    chain_->SetBranchAddress("TTStubs_modId"    , &(vb_modId));
    chain_->SetBranchAddress("TTStubs_nhits"    , &(vb_nhits));
    //chain_->SetBranchAddress("TTStubs_simPt"    , &(vb_simPt));
    //chain_->SetBranchAddress("TTStubs_simEta"   , &(vb_simEta));
    //chain_->SetBranchAddress("TTStubs_simPhi"   , &(vb_simPhi));
    //chain_->SetBranchAddress("TTStubs_trkId"    , &(vb_trkId));

    // Allocate memory
    allPatterns_map_.clear();
    if ((size_t) nEvents_ >= allPatterns_map_.max_size()) {
        std::cout << Error() << "Number of events is more than the max_size of a std::map." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // Loop over all events

    // Containers are declared outside the event loop to avoid memory allocations
    std::vector<id_type> stubLayers;
    pattern_type patt;
    pattern_type pattEmpty;  // for sanity check

    std::map<unsigned, unsigned> towerCountMap;

    float bankSize_f = 0., bankOldSize_f = 0.;

    int nRead = 0, nKept = 0;
    unsigned ievt_step = 0;
    for (long long ievt=0; ievt<nEvents_; ++ievt, ++ievt_step) {
        Long64_t local_entry = chain_->LoadTree(ievt);  // for TChain
        if (local_entry < 0)  break;
        chain_->GetEntry(ievt);

        unsigned nstubs = vb_modId->size();
        if (verbose_>1 && ievt_step == 100000) {
            // Coverage info
            bankSize_f = allPatterns_map_.size();
            coverage_ = 1. - (bankSize_f - bankOldSize_f) / 100000.;
            std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, # patterns: %7.0f, coverage: %7.5f", ievt, nKept, bankSize_f, coverage_) << std::endl;

            bankOldSize_f = bankSize_f;
            ievt_step -= 100000;
        }

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            ++nRead;
            continue;
        }

        // _____________________________________________________________________
        // Start generating patterns
        bool keep = true;

        stubLayers.clear();
        patt.fill(0);
        patt.at(6) = arbiter_ -> superstrip(25, 0, 0, 0, 0);  // turn off for now
        patt.at(7) = arbiter_ -> superstrip(26, 0, 0, 0, 0);  // turn off for now

        // Check min # of layers
        bool require = (nstubs >= MIN_NGOODSTUBS);
        if (!require)
            keep = false;

        // Quick loop over reconstructed stubs
        id_type moduleId, lay, lad, mod, col, row;  // declare the usual suspects
        for (unsigned l=0; (l<nstubs) && keep; ++l) {
            moduleId = vb_modId->at(l);

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
        towerCountMap.clear();

        // Loop over reconstructed stubs
        for (unsigned k=0, l=0; (k<nLayers_) && keep; ++k) {
            // Fake superstrip
            if (indices.at(k) == 999999) {
                // Arbitrarily put fake superstrip in layer 27
                patt.at(k) = arbiter_ -> superstrip(27, 0, 0, 0, 0);
                continue;
            }

            // Real superstrip (tracker only)
            assert(k < nstubs && indices.at(k) < nstubs);
            l = indices.at(k);

            // Break moduleId into lay, lad, mod
            moduleId = vb_modId->at(l);
            lay = decodeLayer(moduleId);
            lad = decodeLadder(moduleId);
            mod = decodeModule(moduleId);

            // col <-- coordy, row <-- coordx
            // use half-strip unit
            col = halfStripRound(vb_coordy->at(l));
            row = halfStripRound(vb_coordx->at(l));

            // Find superstrip address
            const id_type& ssId = arbiter_ -> superstrip(lay, lad, mod, col, row);
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
            ++allPatterns_map_[patt];
            ++nKept;

            if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " patt: " << patt << std::endl;
        }
        ++nRead;
    }
    if (verbose_)  std::cout << Info() << Form("Read: %7i, kept: %7i, # patterns: %7lu", nRead, nKept, allPatterns_map_.size()) << std::endl;

    // Keep this number to calculate sorted coverage
    coverage_count_ = nKept;

    // Convert map to vector of pairs
    const unsigned origSize = allPatterns_map_.size();
    //allPatterns_map_pairs_.reserve(allPatterns_map_.size());  // can cause bad_alloc
    //allPatterns_map_pairs_.insert(allPatterns_map_pairs_.end(), allPatterns_map_.begin(), allPatterns_map_.end());

    for (std::map<pattern_type, unsigned>::const_iterator it = allPatterns_map_.begin();
         it != allPatterns_map_.end(); ) {  // should not cause bad_alloc
        allPatterns_map_pairs_.push_back(*it);
        it = allPatterns_map_.erase(it);
    }
    assert(allPatterns_map_pairs_.size() == origSize);

    // Clear the map and release memory
    std::map<pattern_type, unsigned> mapEmpty;
    allPatterns_map_.clear();
    allPatterns_map_.swap(mapEmpty);

    // Sort by frequency
    std::stable_sort(allPatterns_map_pairs_.begin(), allPatterns_map_pairs_.end(), sortByFrequency);
    assert(allPatterns_map_pairs_.front().second <= MAX_FREQUENCY);

    if (verbose_>2) {
        for (unsigned i=0; i<allPatterns_map_pairs_.size(); ++i) {
            const auto& pair = allPatterns_map_pairs_.at(i);
            std::cout << Debug() << "... patt: " << i << "  " << pair.first << " freq: " << pair.second << std::endl;
        }
    }

    unsigned highest_freq = allPatterns_map_pairs_.size() ? allPatterns_map_pairs_.front().second : 0;
    if (verbose_)  std::cout << Info() << "Generated " << allPatterns_map_pairs_.size() << " patterns, highest freq: " << highest_freq << std::endl;

    return 0;
}


// _____________________________________________________________________________
// Output patterns into a TTree
int PatternGenerator::writePatterns_map(TString out) {
    if (!out.EndsWith(".root")) {
        std::cout << Error() << "Output filename must be .root" << std::endl;
        return 1;
    }

    const long long nentries = allPatterns_map_pairs_.size();
    if (verbose_)  std::cout << Info() << "Recreating " << out << " with " << nentries << " patterns." << std::endl;
    TFile* tfile = TFile::Open(out, "RECREATE");
    TTree* ttree = new TTree(bankName_, "");

    std::auto_ptr<count_type>             frequency       (new count_type(0));
    std::auto_ptr<std::vector<id_type> >  superstripIds   (new std::vector<id_type>());

    ttree->Branch("frequency"       , &(*frequency));
    ttree->Branch("superstripIds"   , &(*superstripIds));

    // _________________________________________________________________________
    // Loop over all patterns

    unsigned integralFrequency = 0;
    float sortedCoverage = 0;
    count_type oldFrequency = MAX_FREQUENCY;

    unsigned ievt_step = 0;
    for (long long ievt=0; ievt<nentries; ++ievt, ++ievt_step) {
        if (verbose_>1 && ievt_step == 100000) {
            sortedCoverage = float(integralFrequency) / coverage_count_ * coverage_;
            std::cout << Debug() << Form("... Writing event: %7lld, sorted coverage: %7.5f", ievt, sortedCoverage) << std::endl;

            ievt_step -= 100000;
        }

        *frequency = allPatterns_map_pairs_.at(ievt).second;;

        // make sure it is indeed sorted
        assert(oldFrequency >= *frequency);
        oldFrequency = *frequency;
        integralFrequency += *(frequency);

        if (*frequency < minFrequency_)  // cut off
            break;

        superstripIds->clear();
        const pattern_type& patt = allPatterns_map_pairs_.at(ievt).first;
        for (unsigned i=0; i<patt.size(); ++i) {
            superstripIds->push_back(patt.at(i));
        }

        ttree->Fill();
    }
    assert(ttree->GetEntries() == nentries);

    // _________________________________________________________________________
    // Also save the trigger tower maps
    TTree* ttree2 = new TTree("triggerTower", "");
    std::map<unsigned, std::vector<unsigned> > * ptr_map1 = &triggerTowerMap_;
    std::map<unsigned, std::vector<unsigned> > * ptr_map2 = &triggerTowerReverseMap_;
    ttree2->Branch("triggerTowerMap", ptr_map1);
    ttree2->Branch("triggerTowerReverseMap", ptr_map2);
    ttree2->Fill();
    assert(ttree2->GetEntries() == 1);

    tfile->Write();
    delete ttree2;
    delete ttree;
    delete tfile;
    return 0;
}


// _____________________________________________________________________________
// Make the patterns
int PatternGenerator::makePatterns_fas() {
    long long nentries = chain_->GetEntries();
    if (nentries <= 0) {
        std::cout << Error() << "Input source has zero entry." << std::endl;
        return 1;
    }
    if (nEvents_ > nentries)
        nEvents_ = nentries;

    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events." << std::endl;

    // For reading
    //std::vector<float> *          vb_x          = 0;
    //std::vector<float> *          vb_y          = 0;
    //std::vector<float> *          vb_z          = 0;
    //std::vector<float> *          vb_r          = 0;
    //std::vector<float> *          vb_phi        = 0;
    std::vector<float> *          vb_coordx     = 0;
    std::vector<float> *          vb_coordy     = 0;
    std::vector<float> *          vb_roughPt    = 0;
    std::vector<unsigned> *       vb_modId      = 0;
    std::vector<unsigned> *       vb_nhits      = 0;
    //std::vector<float> *          vb_simPt      = 0;
    //std::vector<float> *          vb_simEta     = 0;
    //std::vector<float> *          vb_simPhi     = 0;
    //std::vector<int> *            vb_trkId      = 0;

    chain_->SetBranchStatus("*"                 , 0);
    //chain_->SetBranchStatus("TTStubs_x"         , 1);
    //chain_->SetBranchStatus("TTStubs_y"         , 1);
    //chain_->SetBranchStatus("TTStubs_z"         , 1);
    //chain_->SetBranchStatus("TTStubs_r"         , 1);
    //chain_->SetBranchStatus("TTStubs_phi"       , 1);
    chain_->SetBranchStatus("TTStubs_coordx"    , 1);
    chain_->SetBranchStatus("TTStubs_coordy"    , 1);
    chain_->SetBranchStatus("TTStubs_roughPt"   , 1);
    chain_->SetBranchStatus("TTStubs_modId"     , 1);
    chain_->SetBranchStatus("TTStubs_nhits"     , 1);
    //chain_->SetBranchStatus("TTStubs_simPt"     , 1);
    //chain_->SetBranchStatus("TTStubs_simEta"    , 1);
    //chain_->SetBranchStatus("TTStubs_simPhi"    , 1);
    //chain_->SetBranchStatus("TTStubs_trkId"     , 1);

    //chain_->SetBranchAddress("TTStubs_x"        , &(vb_x));
    //chain_->SetBranchAddress("TTStubs_y"        , &(vb_y));
    //chain_->SetBranchAddress("TTStubs_z"        , &(vb_z));
    //chain_->SetBranchAddress("TTStubs_r"        , &(vb_r));
    //chain_->SetBranchAddress("TTStubs_phi"      , &(vb_phi));
    chain_->SetBranchAddress("TTStubs_coordx"   , &(vb_coordx));
    chain_->SetBranchAddress("TTStubs_coordy"   , &(vb_coordy));
    chain_->SetBranchAddress("TTStubs_roughPt"  , &(vb_roughPt));
    chain_->SetBranchAddress("TTStubs_modId"    , &(vb_modId));
    chain_->SetBranchAddress("TTStubs_nhits"    , &(vb_nhits));
    //chain_->SetBranchAddress("TTStubs_simPt"    , &(vb_simPt));
    //chain_->SetBranchAddress("TTStubs_simEta"   , &(vb_simEta));
    //chain_->SetBranchAddress("TTStubs_simPhi"   , &(vb_simPhi));
    //chain_->SetBranchAddress("TTStubs_trkId"    , &(vb_trkId));

    // Allocate memory
    allPatterns_fas_.reserve(nEvents_);

    // _________________________________________________________________________
    // Loop over all events

    // Containers are declared outside the event loop to avoid memory allocations
    std::vector<id_type> stubLayers;
    pattern_type patt;
    pattern_type pattEmpty;  // for sanity check

    std::map<unsigned, unsigned> towerCountMap;

    float bankSize_f = 0., bankOldSize_f = 0.;

    int nRead = 0, nKept = 0;
    unsigned ievt_step = 0;
    for (long long ievt=0; ievt<nEvents_; ++ievt, ++ievt_step) {
        Long64_t local_entry = chain_->LoadTree(ievt);  // for TChain
        if (local_entry < 0)  break;
        chain_->GetEntry(ievt);

        unsigned nstubs = vb_modId->size();
        if (verbose_>1 && ievt_step == 100000) {
            // Coverage info
            bankSize_f = allPatterns_fas_.size();
            coverage_ = 1. - (bankSize_f - bankOldSize_f) / 100000.;
            std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, # patterns: %7.0f, coverage: %7.5f", ievt, nKept, bankSize_f, coverage_) << std::endl;

            bankOldSize_f = bankSize_f;
            ievt_step -= 100000;
        }

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            ++nRead;
            continue;
        }

        // _____________________________________________________________________
        // Start generating patterns
        bool keep = true;

        stubLayers.clear();
        patt.fill(0);
        patt.at(6) = arbiter_ -> superstrip(25, 0, 0, 0, 0);  // turn off for now
        patt.at(7) = arbiter_ -> superstrip(26, 0, 0, 0, 0);  // turn off for now

        // Check min # of layers
        bool require = (nstubs >= MIN_NGOODSTUBS);
        if (!require)
            keep = false;

        // Quick loop over reconstructed stubs
        id_type moduleId, lay, lad, mod, col, row;  // declare the usual suspects
        for (unsigned l=0; (l<nstubs) && keep; ++l) {
            moduleId = vb_modId->at(l);

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
        towerCountMap.clear();

        // Loop over reconstructed stubs
        for (unsigned k=0, l=0; (k<nLayers_) && keep; ++k) {
            // Fake superstrip
            if (indices.at(k) == 999999) {
                // Arbitrarily put fake superstrip in layer 27
                patt.at(k) = arbiter_ -> superstrip(27, 0, 0, 0, 0);
                continue;
            }

            // Real superstrip (tracker only)
            assert(k < nstubs && indices.at(k) < nstubs);
            l = indices.at(k);

            // Break moduleId into lay, lad, mod
            moduleId = vb_modId->at(l);
            lay = decodeLayer(moduleId);
            lad = decodeLadder(moduleId);
            mod = decodeModule(moduleId);

            // col <-- coordy, row <-- coordx
            // use half-strip unit
            col = halfStripRound(vb_coordy->at(l));
            row = halfStripRound(vb_coordx->at(l));

            // Find superstrip address
            const id_type& ssId = arbiter_ -> superstrip(lay, lad, mod, col, row);
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
    if (verbose_)  std::cout << Info() << Form("Read: %7i, kept: %7i, # patterns: %7u", nRead, nKept, allPatterns_fas_.size()) << std::endl;

    // Keep this number to calculate sorted coverage
    coverage_count_ = nKept;

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
    if (!out.EndsWith(".root")) {
        std::cout << Error() << "Output filename must be .root" << std::endl;
        return 1;
    }

    const long long nentries = allPatterns_fas_.size();
    if (verbose_)  std::cout << Info() << "Recreating " << out << " with " << nentries << " patterns." << std::endl;
    TFile* tfile = TFile::Open(out, "RECREATE");

    // _________________________________________________________________________
    // Save trigger tower maps
    TTree* ttree2 = new TTree("triggerTower", "");
    std::map<unsigned, std::vector<unsigned> > * ptr_map1 = &triggerTowerMap_;
    std::map<unsigned, std::vector<unsigned> > * ptr_map2 = &triggerTowerReverseMap_;
    ttree2->Branch("triggerTowerMap", ptr_map1);
    ttree2->Branch("triggerTowerReverseMap", ptr_map2);
    ttree2->Fill();
    assert(ttree2->GetEntries() == 1);

    // _________________________________________________________________________
    // Save pattern bank
    TTree* ttree = new TTree(bankName_, "");
    std::auto_ptr<count_type>             frequency       (new count_type(0));
    std::auto_ptr<std::vector<id_type> >  superstripIds   (new std::vector<id_type>());

    ttree->Branch("frequency"       , &(*frequency));
    ttree->Branch("superstripIds"   , &(*superstripIds));

    // _________________________________________________________________________
    // Loop over all patterns

    unsigned integralFrequency = 0;
    float sortedCoverage = 0;
    count_type oldFrequency = MAX_FREQUENCY;

    unsigned ievt_step = 0;
    for (long long ievt=0; ievt<nentries; ++ievt, ++ievt_step) {
        if (verbose_>1 && ievt_step == 100000) {
            sortedCoverage = float(integralFrequency) / coverage_count_ * coverage_;
            std::cout << Debug() << Form("... Writing event: %7lld, sorted coverage: %7.5f", ievt, sortedCoverage) << std::endl;

            ievt_step -= 100000;
        }

        const fas::lean_table3::return_type& ret = allPatterns_fas_.at(ievt);
        *frequency = ret.count;

        // make sure it is indeed sorted
        assert(oldFrequency >= *frequency);
        oldFrequency = *frequency;
        integralFrequency += *(frequency);

        if (*frequency < minFrequency_)  // cut off
            break;

        superstripIds->clear();
        for (id_type * it = ret.begin; it != ret.end; ++it) {
            superstripIds->push_back(*it);
        }

        ttree->Fill();
    }
    assert(ttree->GetEntries() == nentries);


    tfile->Write();
    delete ttree2;
    delete ttree;
    delete tfile;
    return 0;
}


// _____________________________________________________________________________
// Private functions
// none


// _____________________________________________________________________________
// Main driver
int PatternGenerator::run(TString out, TString src, TString layout) {
    gROOT->ProcessLine("#include <vector>");  // how is it not loaded?

    int exitcode = 0;
    Timing(1);

    exitcode = readTriggerTowerFile(layout);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = readFile(src);
    if (exitcode)  return exitcode;
    Timing();

    bool use_fas = true;
    if (use_fas) {
        exitcode = makePatterns_fas();
        if (exitcode)  return exitcode;
        Timing();

        exitcode = writePatterns_fas(out);
        if (exitcode)  return exitcode;
        Timing();

    } else {
        exitcode = makePatterns_map();
        if (exitcode)  return exitcode;
        Timing();

        exitcode = writePatterns_map(out);
        if (exitcode)  return exitcode;
        Timing();
    }

    chain_->Reset();
    return exitcode;
}
