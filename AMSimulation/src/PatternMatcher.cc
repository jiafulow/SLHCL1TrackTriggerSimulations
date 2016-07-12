#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternMatcher.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/PatternBankReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubPlusTPReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"
#include <bitset>


namespace {
// Join 'layer' and 'superstrip' into one number
unsigned simpleHash(unsigned layer, unsigned nss, unsigned ss) {
    return layer * nss + ss;
}

unsigned simpleHashUndo(unsigned layer, unsigned nss, unsigned ssh) {
    return ssh % nss;
}

unsigned simpleHashNbins(unsigned nlayers, unsigned nss) {
    return simpleHash(nlayers, nss, 0);
}
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
    if (npatterns > po_.maxPatterns)
        npatterns = po_.maxPatterns;
    assert(npatterns > 0);

    // Setup hit buffer
    const unsigned nss = arbiter_ -> nsuperstripsPerLayer();

    if (hitBuffer_.init(simpleHashNbins(po_.nLayers, nss))) {
        std::cout << Error() << "Failed to initialize HitBuffer." << std::endl;
        return 1;
    }

    // Setup associative memory
    if (associativeMemory_.init(npatterns)) {
        std::cout << Error() << "Failed to initialize AssociativeMemory." << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << "Assume " << nss << " possible superstrips per layer." << std::endl;

    // _________________________________________________________________________
    // Load the patterns

    pattern_type pattHash;
    pattHash.fill(0);
    float pattInvPt = 0.;

    for (long long ipatt=0; ipatt<npatterns; ++ipatt) {
        pbreader.getPattern(ipatt);
        if (pbreader.pb_frequency < po_.minFrequency)
            break;

        assert(pbreader.pb_superstripIds->size() == po_.nLayers);

        if (verbose_>3) {
            for (unsigned i=0; i<pbreader.pb_superstripIds->size(); ++i) {
                std::cout << Debug() << "... patt: " << ipatt << "  ";
                std::copy(pbreader.pb_superstripIds->begin(), pbreader.pb_superstripIds->end(),
                          std::ostream_iterator<unsigned>(std::cout, " "));
                std::cout << " freq: " << (unsigned) pbreader.pb_frequency << std::endl;
            }
        }

        // Fill the associative memory
        pbreader.getPatternInvPt(ipatt, pattInvPt);

        //associativeMemory_.insert(pbreader.pb_superstripIds->begin(), pbreader.pb_superstripIds->end(), pattInvPt);

        // Fill the associative memory, after hashing
        pattHash.fill(0);
        for (unsigned layer=0; layer<po_.nLayers; ++layer) {
            const unsigned ssId     = pbreader.pb_superstripIds->at(layer);
            const unsigned ssIdHash = simpleHash(layer, nss, ssId);
            pattHash.at(layer) = ssIdHash;
        }

        associativeMemory_.insert(pattHash, pattInvPt);
    }

    associativeMemory_.freeze();
    assert(associativeMemory_.size() == npatterns);

    if (verbose_)  std::cout << Info() << "Successfully loaded " << npatterns << " patterns." << std::endl;

    return 0;
}

// _____________________________________________________________________________
int PatternMatcher::makeRoads(TString src, TString out) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and matching patterns." << std::endl;

    // _________________________________________________________________________
    // Get trigger tower reverse map
    const std::map<unsigned, bool>& ttrmap = ttmap_ -> getTriggerTowerReverseMap(po_.tower);


    // _________________________________________________________________________
    // For reading
    TTStubPlusTPReader reader(verbose_);
    if (reader.init(src)) {
        std::cout << Error() << "Failed to initialize TTStubPlusTPReader." << std::endl;
        return 1;
    }

    // For writing
    TTRoadWriter writer(verbose_);
    if (writer.init(reader.getChain(), out, prefixRoad_, suffix_)) {
        std::cout << Error() << "Failed to initialize TTRoadWriter." << std::endl;
        return 1;
    }


    // _________________________________________________________________________
    // Loop over all events

    const unsigned nss = arbiter_ -> nsuperstripsPerLayer();

    // Containers
    std::vector<TTRoad> roads;
    roads.reserve(300);

    std::vector<std::string> stubs_bitString;
    std::vector<unsigned>    stubs_superstripId;

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
        // Skip stubs

        std::vector<bool> stubsNotInTower;  // true: not in this trigger tower
        for (unsigned istub=0; istub<nstubs; ++istub) {
            unsigned moduleId = reader.vb_modId   ->at(istub);
            float    stub_ds  = reader.vb_trigBend->at(istub);  // in full-strip unit

            // Skip if not in this trigger tower
            bool isNotInTower = (ttrmap.find(moduleId) == ttrmap.end());
            bool isNotInStubWindow = !cutter_->applyCuts(moduleId, stub_ds);

            stubsNotInTower.push_back(isNotInTower || isNotInStubWindow);
        }

        // Null stub information for those that are not in this trigger tower
        reader.nullStubs(stubsNotInTower);


        // _____________________________________________________________________
        // Skip tracking particles

        std::vector<bool> trkPartsNotPrimary;  // true: not primary

        const unsigned nparts = reader.vp2_primary->size();
        for (unsigned ipart=0; ipart<nparts; ++ipart) {

            // Skip if not primary
            bool  primary         = reader.vp2_primary->at(ipart);
            int   simCharge       = reader.vp2_charge->at(ipart);
            trkPartsNotPrimary.push_back(!(simCharge!=0 && primary));
        }

        // Null trkPart information for those that are not primary
        reader.nullParticles(trkPartsNotPrimary);


        // _____________________________________________________________________
        // Start pattern recognition
        hitBuffer_.reset();

        stubs_bitString.clear();
        stubs_superstripId.clear();

        float conv_r = 0., conv_phi = 0., conv_z = 0.;
        int64_t conv_r_int = 0, conv_phi_int = 0, conv_z_int = 0;
        LocalToGlobal conv_l2g;
        LocalToGlobalInt conv_l2g_int;

        // Loop over reconstructed stubs
        for (unsigned istub=0; istub<nstubs; ++istub) {
            bool isNotInTower = stubsNotInTower.at(istub);
            if (isNotInTower) {
                stubs_bitString.push_back("");
                stubs_superstripId.push_back(0);
                continue;
            }

            unsigned moduleId = reader.vb_modId   ->at(istub);
            float    strip    = reader.vb_coordx  ->at(istub);  // in full-strip unit
            float    segment  = reader.vb_coordy  ->at(istub);  // in full-strip unit

            float    stub_r   = reader.vb_r       ->at(istub);
            float    stub_phi = reader.vb_phi     ->at(istub);
            float    stub_z   = reader.vb_z       ->at(istub);
            float    stub_ds  = reader.vb_trigBend->at(istub);  // in full-strip unit

            // Do local-to-global conversion
            l2gmap_ -> convert(moduleId, strip, segment, conv_r, conv_phi, conv_z, conv_l2g);

            // Find superstrip ID
            unsigned ssId = 0;
            if (!arbiter_ -> useGlobalCoord()) {  // local coordinates
                ssId = arbiter_ -> superstripLocal(moduleId, strip, segment);

            } else {                              // global coordinates
                if (po_.emu == 0) {
                    ssId = arbiter_ -> superstripGlobal(moduleId, stub_r, stub_phi, stub_z, stub_ds);
                } else {
                    ssId = arbiter_ -> superstripLocal(moduleId, strip, segment, conv_l2g, conv_l2g_int);
                }
            }

            unsigned lay16    = compressLayer(decodeLayer(moduleId));
            unsigned ssIdHash = simpleHash(lay16, nss, ssId);

            // Push into hit buffer
            hitBuffer_.insert(ssIdHash, istub);

            if (verbose_>2) {
                std::cout << Debug() << "... ... stub: " << istub << " moduleId: " << moduleId << " strip: " << strip << " segment: " << segment << " r: " << stub_r << " phi: " << stub_phi << " z: " << stub_z << " ds: " << stub_ds << std::endl;
                std::cout << Debug() << "... ... stub: " << istub << " ssId: " << ssId << " ssIdHash: " << ssIdHash << std::endl;
            }

            if (po_.emu == 0) {
                stubs_bitString.push_back("");
                stubs_superstripId.push_back(0);
            } else {
                l2gmap_ -> convert(moduleId, strip, segment, conv_r, conv_phi, conv_z, conv_l2g);
                l2gmap_ -> convertInt(moduleId, strip, segment, po_.tower, conv_l2g, conv_r_int, conv_phi_int, conv_z_int, conv_l2g_int);

                // All together this is a 66 bit string for every stub (after AM) going to the track fitter
                // emulator which Marco will interface
                //   1 bit for data_valid (in simulation we can just set it to 1 for every stub)
                //   18 bits for phi with range of plus/minus 1 radian
                //   18 bits for R  with range of plus/minus 1024 cm
                //   18 bits for z with  range of plus/minus 1024 cm
                //   4 bits for stub bend info
                //   7 bits for the strip ID within the module for radial conversion.
                int bend_4b = int(std::round(stub_ds)) >> 1;
                int strip_7b = (halfStripRound(strip)) >> 4;

                std::string bitString = "";
                bitString += std::bitset<1>(int(1)).to_string();
                bitString += std::bitset<18>(conv_phi_int).to_string();
                bitString += std::bitset<18>(conv_r_int).to_string();
                bitString += std::bitset<18>(conv_z_int).to_string();
                bitString += std::bitset<4>(bend_4b).to_string();
                bitString += std::bitset<7>(strip_7b).to_string();
                assert(bitString.size() == 66);

                stubs_bitString.push_back(bitString);
                stubs_superstripId.push_back(ssId);
            }
        }

        hitBuffer_.freeze(po_.maxStubs);

        // _____________________________________________________________________
        // Perform associative memory lookup
        const std::vector<unsigned>& firedPatterns = associativeMemory_.lookup(hitBuffer_, po_.nLayers, po_.maxMisses);


        // _____________________________________________________________________
        // Create roads
        roads.clear();

        // Collect stubs
        for (std::vector<unsigned>::const_iterator it = firedPatterns.begin(); it != firedPatterns.end(); ++it) {
            // Create and set TTRoad
            TTRoad aroad;
            aroad.patternRef   = (*it);
            aroad.tower        = po_.tower;
            aroad.nstubs       = 0;
            aroad.patternInvPt = 0.;

            // Retrieve the superstripIds and other attributes
            pattern_type pattHash;
            associativeMemory_.retrieve(aroad.patternRef, pattHash, aroad.patternInvPt);

            aroad.superstripIds.clear();
            aroad.stubRefs.clear();

            aroad.superstripIds.resize(po_.nLayers);
            aroad.stubRefs.resize(po_.nLayers);

            for (unsigned layer=0; layer<po_.nLayers; ++layer) {
                const unsigned ssIdHash = pattHash.at(layer);
                const unsigned ssId     = simpleHashUndo(layer, nss, ssIdHash);

                if (hitBuffer_.isHit(ssIdHash)) {
                    const std::vector<unsigned>& stubRefs = hitBuffer_.getHits(ssIdHash);
                    aroad.superstripIds.at(layer) = ssId;
                    aroad.stubRefs     .at(layer) = stubRefs;
                    aroad.nstubs                 += stubRefs.size();

                } else {
                    aroad.superstripIds.at(layer) = ssId;
                }
            }

            roads.push_back(aroad);  // save aroad

            if (verbose_>2)  std::cout << Debug() << "... ... road: " << roads.size() - 1 << " " << aroad << std::endl;

            if (roads.size() >= (unsigned) po_.maxRoads)
                break;
        }

        if (! roads.empty())
            ++nKept;

        assert(reader.vb_modId->size() == stubs_bitString.size());
        assert(reader.vb_modId->size() == stubs_superstripId.size());

        writer.fill(roads, stubs_bitString, stubs_superstripId);
        ++nRead;
    }

    if (nRead == 0) {
        std::cout << Error() << "Failed to read any event." << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << Form("Read: %7ld, triggered: %7ld", nRead, nKept) << std::endl;

    long long nentries = writer.writeTree();
    assert(nentries == nRead);

    return 0;
}


// _____________________________________________________________________________
// Main driver
int PatternMatcher::run() {
    int exitcode = 0;
    Timing(1);

    exitcode = loadPatterns(po_.bankfile);
    if (exitcode)  return exitcode;
    Timing();

    exitcode = makeRoads(po_.input, po_.output);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
