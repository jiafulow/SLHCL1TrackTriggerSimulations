#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternMatcher.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"

namespace {
// Simple joining
unsigned simpleHash(unsigned layer, unsigned nss, unsigned ss) {
    return layer * nss + ss;
}

unsigned simpleHashNbins(unsigned nlayers, unsigned nss) {
    return simpleHash(nlayers, nss, 0);
}
}


// _____________________________________________________________________________
int PatternMatcher::setupTriggerTower(TString datadir) {
    TString csvfile1 = datadir + "trigger_sector_map.csv";
    TString csvfile2 = datadir + "trigger_sector_boundaries.csv";

    try {
        ttmap_ -> readTriggerTowerMap(csvfile1);

    } catch (const std::invalid_argument& e) {
        std::cout << Error() << "Failed to parse: " << csvfile1 << ". What: " << e.what() << std::endl;
        return 1;
    }

    try {
        ttmap_ -> readTriggerTowerBoundaries(csvfile2);

    } catch (const std::invalid_argument& e) {
        std::cout << Error() << "Failed to parse: " << csvfile2 << ". What: " << e.what() << std::endl;
        return 1;
    }

    //ttmap_ -> print();
    return 0;
}

// _____________________________________________________________________________
int PatternMatcher::setupSuperstrip() {
    try {
        // FIXME: will only work for one trigger tower
        arbiter_ -> setDefinition(po_.superstrip, po_.tower, ttmap_);

    } catch (const std::invalid_argument& e) {
        std::cout << Error() << "Failed to set definition: " << po_.superstrip << ". What: " << e.what() << std::endl;
        return 1;
    }

    //arbiter_ -> print();
    return 0;
}

// _____________________________________________________________________________
int PatternMatcher::loadPatterns(TString bank) {
    if (verbose_)  std::cout << Info() << "Loading patterns from " << bank << std::endl;

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
    assert(npatterns > 0);

    // Allocate memory
    patternBank_.clear();
    patternBank_.resize(npatterns);

    // _________________________________________________________________________
    // Load the patterns

    for (long long ipatt=0; ipatt<npatterns; ++ipatt) {
        pbreader.getPattern(ipatt);
        if (pbreader.pb_frequency < minFrequency_)
            break;

        assert(pbreader.pb_superstripIds->size() == po_.nLayers);

        // Fill the patternBank, which is just a big vector
        std::copy(pbreader.pb_superstripIds->begin(), pbreader.pb_superstripIds->end(), patternBank_.at(ipatt).begin());

        if (verbose_>3) {
            for (unsigned i=0; i<pbreader.pb_superstripIds->size(); ++i) {
                std::cout << Debug() << "... patt: " << ipatt << "  ";
                std::copy(pbreader.pb_superstripIds->begin(), pbreader.pb_superstripIds->end(),
                          std::ostream_iterator<unsigned>(std::cout, " "));
                std::cout << " freq: " << (unsigned) pbreader.pb_frequency << std::endl;
            }
        }
    }

    if ((unsigned) npatterns == patternBank_.size()) {
        if (verbose_)  std::cout << Info() << "Successfully loaded " << npatterns << " patterns." << std::endl;
    } else {
        std::cout << Error() << "Failed to load all patterns: " << patternBank_.size() << " vs " << npatterns << std::endl;
        return 1;
    }

    return 0;
}

// _____________________________________________________________________________
int PatternMatcher::makeRoads(TString src, TString out) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and matching patterns." << std::endl;

    // _________________________________________________________________________
    // Get trigger tower reverse map
    const std::map<unsigned, bool>& ttrmap = ttmap_ -> getTriggerTowerReverseMap(po_.tower);

    // Setup hit buffer
    const unsigned nss = arbiter_ -> nsuperstripsPerLayer();
    hitBuffer_bool_.resize(simpleHashNbins(po_.nLayers, nss));

    if (verbose_)  std::cout << Info() << "Assume " << nss << " possible superstrips per layer." << std::endl;

    // _________________________________________________________________________
    // For reading
    TTStubReader reader(verbose_);
    if (reader.init(src)) {
        std::cout << Error() << "Failed to initialize TTStubReader." << std::endl;
        return 1;

    } else {
        // Only read certain branches
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
        tchain->SetBranchStatus("TTStubs_trigBend"  , 1);
      //tchain->SetBranchStatus("TTStubs_roughPt"   , 1);
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
    std::vector<TTRoad> roads;
    roads.reserve(300);

    // Bookkeepers
    long int nRead = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%100==0)  std::cout << Debug() << Form("... Processing event: %7lld, triggering: %7ld", ievt, nKept) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            ++nRead;
            writer.fill(std::vector<TTRoad>());
            continue;
        }

        if (nstubs > 500000) {
            std::cout << Error() << "Way too many stubs: " << nstubs << std::endl;
            return 1;
        }


        // _____________________________________________________________________
        // Start pattern recognition
        hitBuffer_.clear();
        std::fill(hitBuffer_bool_.begin(), hitBuffer_bool_.end(), false);

        std::vector<bool> stubsNotInTower;  // true: not in this trigger tower

        // Loop over reconstructed stubs
        for (unsigned istub=0; istub<nstubs; ++istub) {
            unsigned moduleId = reader.vb_modId   ->at(istub);

            // Skip if not in this trigger tower
            bool isNotInTower = (ttrmap.find(moduleId) == ttrmap.end());
            stubsNotInTower.push_back(isNotInTower);

            if (isNotInTower)
                continue;

            float    strip    = reader.vb_coordx  ->at(istub);  // in full-strip unit
            float    segment  = reader.vb_coordy  ->at(istub);  // in full-strip unit

            float    stub_r   = reader.vb_r       ->at(istub);
            float    stub_phi = reader.vb_phi     ->at(istub);
            float    stub_z   = reader.vb_z       ->at(istub);
            float    stub_ds  = reader.vb_trigBend->at(istub);  // in full-strip unit

            // Find superstrip ID
            unsigned ssId = 0;
            if (!arbiter_ -> useGlobalCoord()) {  // local coordinates
                ssId = arbiter_ -> superstripLocal(moduleId, strip, segment);

            } else {                              // global coordinates
                ssId = arbiter_ -> superstripGlobal(moduleId, stub_r, stub_phi, stub_z, stub_ds);
            }

            unsigned lay16    = compressLayer(decodeLayer(moduleId));
            unsigned ssIdHash = simpleHash(lay16, nss, ssId);

            // Push into hit buffer
            hitBuffer_[ssIdHash].push_back(istub);  // a map
            hitBuffer_bool_[ssIdHash] = true;       // a hash table

            if (verbose_>2) {
                std::cout << Debug() << "... ... stub: " << istub << " moduleId: " << moduleId << " strip: " << strip << " segment: " << segment << " r: " << stub_r << " phi: " << stub_phi << " z: " << stub_z << " ds: " << stub_ds << std::endl;
                std::cout << Debug() << "... ... stub: " << istub << " ssId: " << ssId << " ssIdHash: " << ssIdHash << std::endl;
            }
        }

        // Null stub information for those that are not in this trigger tower
        reader.nullStubs(stubsNotInTower);

        // Limit the number of stubs per superstrip
        for (std::map<unsigned, std::vector<unsigned> >::iterator it=hitBuffer_.begin();
             it!=hitBuffer_.end(); ++it) {
            if (it->second.size() > (unsigned) maxStubs_)
                it->second.resize(maxStubs_);
        }


        // _____________________________________________________________________
        // Perform direct lookup
        std::vector<unsigned> firedPatterns;

        for (std::vector<pattern_type>::const_iterator itpatt = patternBank_.begin();
             itpatt != patternBank_.end(); ++itpatt) {
            int nMisses = 0;

            unsigned layer = po_.nLayers - 1;  // start from the last layer
            for (pattern_type::const_reverse_iterator itlayer = itpatt->rend() - po_.nLayers;
                 itlayer != itpatt->rend(); ++itlayer, --layer) {
                const unsigned ssId     = *itlayer;
                const unsigned ssIdHash = simpleHash(layer, nss, ssId);

                if (!hitBuffer_bool_.at(ssIdHash))
                    ++nMisses;

                // Skip if more misses than allowed
                if (nMisses > maxMisses_)
                    break;
            }
            if (nMisses <= maxMisses_)
                firedPatterns.push_back(itpatt - patternBank_.begin());
        }


        // _____________________________________________________________________
        // Create roads
        roads.clear();

        // Collect stubs
        std::map<unsigned, std::vector<unsigned> >::const_iterator found;
        std::vector<unsigned>::const_iterator itfound;

        for (std::vector<unsigned>::const_iterator it = firedPatterns.begin(); it != firedPatterns.end(); ++it) {
            // Create and set TTRoad
            TTRoad aroad;
            aroad.patternRef = (*it);
            aroad.tower      = po_.tower;
            aroad.nstubs     = 0;

            aroad.superstripIds.clear();
            aroad.stubRefs.clear();

            aroad.superstripIds.resize(po_.nLayers);
            aroad.stubRefs.resize(po_.nLayers);

            // Retrieve the pattern to get the superstripIds
            std::vector<pattern_type>::const_iterator itpatt = patternBank_.begin() + (*it);

            unsigned layer = 0;
            for (pattern_type::const_iterator itlayer = itpatt->begin();
                 itlayer != itpatt->begin() + po_.nLayers; ++itlayer, ++layer) {
                const unsigned ssId     = *itlayer;
                const unsigned ssIdHash = simpleHash(layer, nss, ssId);

                found = hitBuffer_.find(ssIdHash);
                if (found != hitBuffer_.end()) {
                    aroad.superstripIds[layer] = found->first;
                    aroad.stubRefs     [layer] = found->second;
                    aroad.nstubs              += found->second.size();

                } else {
                    // ??
                }
            }

            roads.push_back(aroad);  // save aroad

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << roads.size() - 1 << " " << aroad << std::endl;

            if (roads.size() >= (unsigned) maxRoads_)
                break;
        }

        if (! roads.empty())
            ++nKept;

        writer.fill(roads);
        ++nRead;
    }

    if (verbose_)  std::cout << Info() << Form("Read: %7ld, triggered: %7ld", nRead, nKept) << std::endl;

    long long nentries = writer.writeTree();
    assert(nentries == nRead);

    return 0;
}


// _____________________________________________________________________________
// Main driver
int PatternMatcher::run(TString src, TString bank, TString datadir, TString out) {
    int exitcode = 0;
    Timing(1);

    exitcode = setupTriggerTower(datadir);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = setupSuperstrip();
    if (exitcode)  return exitcode;
    Timing();

    exitcode = loadPatterns(bank);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = makeRoads(src, out);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
