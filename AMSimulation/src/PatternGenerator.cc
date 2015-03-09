#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternGenerator.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/CSVFileReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"

static const unsigned MIN_NGOODSTUBS = 3;
static const unsigned MAX_NGOODSTUBS = 8;
static const unsigned MAX_FREQUENCY = 0xffffffff;  // unsigned

namespace {
// Comparator
bool sortByFrequency(const std::pair<pattern_type, unsigned>& lhs, const std::pair<pattern_type, unsigned>& rhs) {
    return lhs.second > rhs.second;
}
}


// _____________________________________________________________________________
int PatternGenerator::setupTriggerTower(TString datadir) {
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
int PatternGenerator::setupSuperstrip() {
    try {
        arbiter_ -> setDefinition(po_.superstrip, po_.tower, ttmap_);

    } catch (const std::invalid_argument& e) {
        std::cout << Error() << "Failed to set definition: " << po_.superstrip << ". What: " << e.what() << std::endl;
        return 1;
    }

    //arbiter_ -> print();
    return 0;
}

// _____________________________________________________________________________
// Make the patterns
int PatternGenerator::makePatterns(TString src) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and generating patterns." << std::endl;

    // _________________________________________________________________________
    // For reading
    TTStubReader reader(verbose_);
    if (reader.init(src, false)) {
        std::cout << Error() << "Failed to initialize TTStubReader." << std::endl;
        return 1;

    } else {
        // Only read certain branches
        TChain* tchain = reader.getChain();
        tchain->SetBranchStatus("*"                 , 0);
        tchain->SetBranchStatus("genParts_pt"       , 1);
      //tchain->SetBranchStatus("genParts_eta"      , 1);
      //tchain->SetBranchStatus("genParts_phi"      , 1);
      //tchain->SetBranchStatus("genParts_vx"       , 1);
      //tchain->SetBranchStatus("genParts_vy"       , 1);
      //tchain->SetBranchStatus("genParts_vz"       , 1);
      //tchain->SetBranchStatus("genParts_charge"   , 1);
      //tchain->SetBranchStatus("TTStubs_x"         , 1);
      //tchain->SetBranchStatus("TTStubs_y"         , 1);
        tchain->SetBranchStatus("TTStubs_z"         , 1);
        tchain->SetBranchStatus("TTStubs_r"         , 1);
      //tchain->SetBranchStatus("TTStubs_eta"       , 1);
        tchain->SetBranchStatus("TTStubs_phi"       , 1);
        tchain->SetBranchStatus("TTStubs_coordx"    , 1);
        tchain->SetBranchStatus("TTStubs_coordy"    , 1);
        tchain->SetBranchStatus("TTStubs_trigBend"  , 1);
      //tchain->SetBranchStatus("TTStubs_roughPt"   , 1);
      //tchain->SetBranchStatus("TTStubs_clusWidth" , 1);
        tchain->SetBranchStatus("TTStubs_modId"     , 1);
      //tchain->SetBranchStatus("TTStubs_tpId"      , 1);
    }

    // _________________________________________________________________________
    // Get trigger tower reverse map
    const std::map<unsigned, bool>& ttrmap = ttmap_ -> getTriggerTowerReverseMap(po_.tower);


    // _________________________________________________________________________
    // Loop over all events

    patternBank_map_.clear();
    pattern_type patt;
    patt.fill(0);

    // Bookkeepers
    float coverage = 0.;
    long int bankSize = 0, bankSizeOld = -100000, nKeptOld = -100000;
    long int nRead = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        // Running estimate of coverage
        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%100000==0) {
            bankSize = patternBank_map_.size();
            coverage = 1.0 - float(bankSize - bankSizeOld) / float(nKept - nKeptOld);

            std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7ld, # patterns: %7ld, coverage: %7.5f", ievt, nKept, bankSize, coverage) << std::endl;

            bankSizeOld = bankSize;
            nKeptOld = nKept;
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

        // Apply track pt requirement
        float simPt = reader.vp_pt->front();
        if (simPt < po_.minPt || po_.maxPt < simPt) {
            ++nRead;
            continue;
        }

        // Apply trigger tower acceptance
        unsigned ngoodstubs = 0;
        for (unsigned istub=0; istub<nstubs; ++istub) {
            unsigned moduleId = reader.vb_modId   ->at(istub);
            if (ttrmap.find(moduleId) != ttrmap.end()) {
                ++ngoodstubs;
            }
        }
        if (ngoodstubs != po_.nLayers) {
            ++nRead;
            continue;
        }
        assert(nstubs == po_.nLayers);


        // _____________________________________________________________________
        // Start generating patterns
        patt.fill(0);

        // Loop over reconstructed stubs
        for (unsigned istub=0; istub<nstubs; ++istub) {
            unsigned moduleId = reader.vb_modId   ->at(istub);
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
            patt.at(istub) = ssId;

            if (verbose_>2) {
                std::cout << Debug() << "... ... stub: " << istub << " moduleId: " << moduleId << " strip: " << strip << " segment: " << segment << " r: " << stub_r << " phi: " << stub_phi << " z: " << stub_z << " ds: " << stub_ds << std::endl;
                std::cout << Debug() << "... ... stub: " << istub << " ssId: " << ssId << std::endl;
            }
        }

        // Insert pattern into the bank
        ++patternBank_map_[patt];

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " patt: " << patt << std::endl;

        ++nKept;
        ++nRead;
    }

    if (verbose_)  std::cout << Info() << Form("Read: %7ld, kept: %7ld, # patterns: %7lu, coverage: %7.5f", nRead, nKept, patternBank_map_.size(), coverage) << std::endl;

    // Save these numbers
    coverage_       = coverage;
    coverage_count_ = nKept;


    // _________________________________________________________________________
    // Sort by frequency

    // Convert map to vector of pairs
    const unsigned origSize = patternBank_map_.size();
    //patternBank_pairs_.reserve(patternBank_map_.size());  // can cause bad_alloc
    //patternBank_pairs_.insert(patternBank_pairs_.end(), patternBank_map_.begin(), patternBank_map_.end());

    for (std::map<pattern_type, unsigned>::const_iterator it = patternBank_map_.begin();
         it != patternBank_map_.end(); ) {  // should not cause bad_alloc
        patternBank_pairs_.push_back(*it);
        it = patternBank_map_.erase(it);
    }
    assert(patternBank_pairs_.size() == origSize);

    // Clear map and release memory
    std::map<pattern_type, unsigned> mapEmpty;
    patternBank_map_.clear();
    patternBank_map_.swap(mapEmpty);

    // Sort by frequency
    std::stable_sort(patternBank_pairs_.begin(), patternBank_pairs_.end(), sortByFrequency);

    if (verbose_>2) {
        for (unsigned i=0; i<patternBank_pairs_.size(); ++i) {
            const auto& pair = patternBank_pairs_.at(i);
            std::cout << Debug() << "... patt: " << i << "  " << pair.first << " freq: " << pair.second << std::endl;
        }
    }

    unsigned highest_freq = patternBank_pairs_.size() ? patternBank_pairs_.front().second : 0;
    if (verbose_)  std::cout << Info() << "Generated " << patternBank_pairs_.size() << " patterns, highest freq: " << highest_freq << std::endl;
    assert(patternBank_pairs_.front().second <= MAX_FREQUENCY);

    return 0;
}


// _____________________________________________________________________________
// Output patterns into a TTree
int PatternGenerator::writePatterns(TString out) {

    // _________________________________________________________________________
    // For writing
    PatternBankWriter writer(verbose_);
    if (writer.init(out)) {
        std::cout << Error() << "Failed to initialize TTRoad writer." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // Save pattern bank statistics
    *(writer.pb_coverage)   = coverage_;
    *(writer.pb_count)      = coverage_count_;
    *(writer.pb_tower)      = po_.tower;
    *(writer.pb_superstrip) = po_.superstrip;
    writer.fillPatternBankInfo();

    // _________________________________________________________________________
    // Save pattern bank
    const long long npatterns = patternBank_pairs_.size();

    // Bookkeepers
    unsigned nKept = 0;
    unsigned freq = MAX_FREQUENCY, oldFreq = MAX_FREQUENCY;
    int n90=0, n95=0, n99=0;

    for (long long ipatt=0; ipatt<npatterns; ++ipatt) {
        if (verbose_>1 && ipatt%100==0) {
            float coverage = float(nKept) / coverage_count_ * coverage_;
            if (coverage < 0.90 + 1e-5)
                n90 = ipatt;
            else if (coverage < 0.95 + 1e-5)
                n95 = ipatt;
            else if (coverage < 0.99 + 1e-5)
                n99 = ipatt;

            std::cout << Debug() << Form("... Writing event: %7lld, sorted coverage: %7.5f", ipatt, coverage) << std::endl;
        }

        freq = patternBank_pairs_.at(ipatt).second;

        // Check whether patterns are indeed sorted by frequency
        assert(oldFreq >= freq);
        oldFreq = freq;
        nKept += freq;

        if (freq < (unsigned) minFrequency_)  // cut off
            break;

        writer.pb_superstripIds->clear();
        const pattern_type& patt = patternBank_pairs_.at(ipatt).first;
        for (unsigned ilayer=0; ilayer<po_.nLayers; ++ilayer) {
            writer.pb_superstripIds->push_back(patt.at(ilayer));
        }
        *(writer.pb_frequency) = freq;
        writer.fillPatternBank();
    }

    long long nentries = writer.writeTree();
    assert(npatterns == nentries);
    assert(coverage_count_ == nKept);

    if (verbose_)  std::cout << Info() << "After sorting by frequency, N(90% cov)=" << n90 << ", N(95% cov)=" << n95 << ", N(99% cov)=" << n99 << std::endl;

    return 0;
}


// _____________________________________________________________________________
// Main driver
int PatternGenerator::run(TString src, TString datadir, TString out) {
    int exitcode = 0;
    Timing(1);

    exitcode = setupTriggerTower(datadir);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = setupSuperstrip();
    if (exitcode)  return exitcode;
    Timing();

    exitcode = makePatterns(src);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = writePatterns(out);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
