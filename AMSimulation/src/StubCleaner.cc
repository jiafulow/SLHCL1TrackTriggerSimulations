#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/StubCleaner.h"

static const unsigned MIN_NGOODSTUBS = 3;
static const unsigned MAX_NGOODSTUBS = 8;

namespace {
// Comparator
bool sortByFloat(const std::pair<unsigned, float>& lhs, const std::pair<unsigned, float>& rhs) {
    return lhs.second < rhs.second;
}

// Comparator
bool sortByUnsignedThenFloat(const std::pair<unsigned, std::pair<unsigned, float> >& lhs,
                             const std::pair<unsigned, std::pair<unsigned, float> >& rhs) {
    // Primary condition
    if (lhs.second.first < rhs.second.first)  return true;
    if (lhs.second.first > rhs.second.first)  return false;

    // Secondary condition
    return lhs.second.second < rhs.second.second;
}

// Insert an element into a specific position in a sorted list
// The algorithm moves every element behind that specific location from i to
// i+1, creating a hole at that location to accomodate the new element.
// 'first' points to the first element in the list
// 'len' is the length of the list
// 'pos' is where the new element is going to be inserted
// 'value' is the value of the new element
template<typename RandomAccessIterator, typename Size, typename T>
void insertSorted(RandomAccessIterator first, Size len, Size pos, const T value) {
    first += len;
    len -= pos;
    while (len>0) {
        *first = std::move(*(first-1));
        --first;
        --len;
    }
    *first = std::move(value);
}

float calcIdealPhi(float simPhi, float simChargeOverPt, float r) {
    static const float mPtFactor = 0.3*3.8*1e-2/2.0;
    //return simPhi - mPtFactor * r * simChargeOverPt;
    return simPhi - std::asin(mPtFactor * r * simChargeOverPt);
}

float calcIdealZ(float simVz, float simCotTheta, float simChargeOverPt, float r) {
    static const float mPtFactor = 0.3*3.8*1e-2/2.0;
    //return simVz + r * simCotTheta;
    return simVz + (1.0 / (mPtFactor * simChargeOverPt) * std::asin(mPtFactor * r * simChargeOverPt)) * simCotTheta;
}

float calcIdealR(float simVz, float simCotTheta, float simChargeOverPt, float z) {
    static const float mPtFactor = 0.3*3.8*1e-2/2.0;
    //return (z - simVz) / simCotTheta;
    return std::abs(1.0 / (mPtFactor * simChargeOverPt) * std::sin(mPtFactor * (z - simVz) / simCotTheta * simChargeOverPt));
}
}


// _____________________________________________________________________________
int StubCleaner::cleanStubs(TString src, TString out) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and cleaning them." << std::endl;

    // _________________________________________________________________________
    // For reading
    TTStubReader reader(verbose_);
    reader.init(src);

    // For writing
    TTStubWriter writer(verbose_);
    writer.init(reader.getChain(), out);

    if (po_.removeOverlap && verbose_>2)  { // RR
        std::map<unsigned,ModuleOverlap>::iterator it_mo;
        std::cout << Info() << momap_->moduleOverlap_map_.size() << std::endl;
        for (it_mo=momap_->moduleOverlap_map_.begin(); it_mo!=momap_->moduleOverlap_map_.end();++it_mo) {
            std::cout << Info() << it_mo->first << "\t" << it_mo->second.layer << std::endl;
        }
    }


    // _________________________________________________________________________
    // Loop over all events

    const int good_tpId = 0;

    // Bookkeepers
    long int nRead = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%50000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7ld", ievt, nKept) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            ++nRead;
            writer.fill();
            continue;
        }

        if (nstubs > 100) {
            std::cout << Error() << "Way too many stubs: " << nstubs << std::endl;
            return 1;
        }

        // _____________________________________________________________________
        // Start cleaning

        // Events that fail don't exit the loop immediately, so that event info
        // can still be printed when verbosity is turned on.
        bool keep = true;

        // Check min # of stubs
        bool require = (nstubs >= MIN_NGOODSTUBS);
        if (!require)
            keep = false;

        // Check sim info
        assert(reader.vp_pt->size() == 1);
        float simPt           = reader.vp_pt->front();
        float simEta          = reader.vp_eta->front();
        float simPhi          = reader.vp_phi->front();
        //float simVx           = reader.vp_vx->front();
        //float simVy           = reader.vp_vy->front();
        float simVz           = reader.vp_vz->front();
        int   simCharge       = reader.vp_charge->front();

        float simCotTheta     = std::sinh(simEta);
        float simChargeOverPt = float(simCharge)/simPt;

        // Apply trigger tower phase space cuts
        int aux_TT = TrackParametersToTT().get_tt(simPhi, simChargeOverPt, simEta, simVz);
        if (aux_TT != (int) po_.tower)
            keep = false;

        // Apply pt, eta, phi requirements
        bool sim = (po_.minPt  <= simPt  && simPt  <= po_.maxPt  &&
                    po_.minEta <= simEta && simEta <= po_.maxEta &&
                    po_.minPhi <= simPhi && simPhi <= po_.maxPhi &&
                    po_.minVz  <= simVz  && simVz  <= po_.maxVz);
        if (!sim)
            keep = false;

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " simPt: " << simPt << " simEta: " << simEta << " simPhi: " << simPhi << " simVz: " << simVz << " simChargeOverPt: " << simChargeOverPt << " aux_TT: " << aux_TT << " keep? " << keep << std::endl;

        // _____________________________________________________________________
        // Remove multiple stubs in one layer

        // Make a vector of pairs, each pair has an id and a 2D (R,D) value,
        // where R is rank based on radius or z coord, D is (dx**2 + dy**2 + dz**2)**(1/2)
        std::vector<std::pair<unsigned, std::pair<unsigned, float> > > vec_index_dist;
        for (unsigned istub=0; (istub<nstubs) && keep; ++istub) {
            int tpId = reader.vb_tpId->at(istub);  // check sim info
            if (tpId != good_tpId)
                continue;

            unsigned moduleId = reader.vb_modId   ->at(istub);

            float    stub_r   = reader.vb_r       ->at(istub);
            float    stub_phi = reader.vb_phi     ->at(istub);
            float    stub_z   = reader.vb_z       ->at(istub);
            float    stub_ds  = reader.vb_trigBend->at(istub);

            // RR removing stubs in the overlapping regions
            if (po_.removeOverlap) {
                float    stub_coordx = reader.vb_coordx->at(istub);
                float    stub_coordy = reader.vb_coordy->at(istub);
                std::map<unsigned,ModuleOverlap>::iterator it_mo = momap_->moduleOverlap_map_.find(moduleId);
                if (it_mo != momap_->moduleOverlap_map_.end()) {
                    float minx = it_mo->second.x1;
                    if (stub_coordx < minx) {
                        if (verbose_>2)  std::cout << Info() << "Removing stub in module " << ievt << "\t" << moduleId << "\t x1: " <<  stub_coordx << std::endl;
                        continue;
                    }
                    float maxx = it_mo->second.x2;
                    if (stub_coordx > maxx) {
                        if (verbose_>2)  std::cout << Info() << "Removing stub in module " << ievt << "\t"  << moduleId << "\t x2: " <<  stub_coordx << std::endl;
                        continue;
                    }
                    float miny = it_mo->second.y1;
                    if (stub_coordy < miny) {
                        if (verbose_>2)  std::cout << Info() << "Removing stub in module " << ievt << "\t"  << moduleId << "\t y1: " <<  stub_coordy << std::endl;
                        continue;
                    }
                    float maxy = it_mo->second.y2;
                    if (stub_coordy > maxy) {
                        if (verbose_>2)  std::cout << Info() << "Removing stub in module " << ievt << "\t"  << moduleId << "\t y2: " <<  stub_coordy << std::endl;
                        continue;
                    }
                }
            }  // end if removeOverlap

            unsigned lay16    = compressLayer(decodeLayer(moduleId));
            assert(lay16 < 16);

            // CUIDADO: simVx and simVy are currently not used in the calculation
            //          therefore d0 is assumed to be zero, and z0 is assumed to be equal to vz
            float idealPhi = calcIdealPhi(simPhi, simChargeOverPt, stub_r);
            float idealZ   = calcIdealZ(simVz, simCotTheta, simChargeOverPt, stub_r);
            float idealR   = stub_r;

            if (lay16 >= 6) {  // for endcap
                idealR     = calcIdealR(simVz, simCotTheta, simChargeOverPt, stub_z);
                idealPhi   = calcIdealPhi(simPhi, simChargeOverPt, idealR);
                idealZ     = stub_z;
            }

            float deltaPhi = stub_phi - idealPhi;
            float deltaZ   = stub_z - idealZ;
            float deltaR   = stub_r - idealR;

            if (verbose_>2)  std::cout << Debug() << "... ... stub: " << istub << " moduleId: " << moduleId << " r: " << stub_r << " phi: " << stub_phi << " z: " << stub_z << " ds: " << stub_ds << " lay16: " << lay16 << " deltaPhi: " << deltaPhi << " deltaR: " << deltaR << " deltaZ: " << deltaZ << std::endl;

            bool picked = picky_ -> applyCuts(lay16, deltaPhi, deltaR, deltaZ);
            if (!po_.picky || (po_.picky && picked) ) {
                unsigned rank = picky_ -> findRank(lay16, stub_r, stub_z);

                float deltaX = stub_r * (std::cos(stub_phi) - std::cos(idealPhi));
                float deltaY = stub_r * (std::sin(stub_phi) - std::sin(idealPhi));
                float dist   = std::sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);

                if (lay16 >= 6) {  // for endcap
                    deltaX = stub_r * std::cos(stub_phi) - idealR * std::cos(idealPhi);
                    deltaY = stub_r * std::sin(stub_phi) - idealR * std::sin(idealPhi);
                    dist   = std::sqrt(deltaX*deltaX + deltaY*deltaY);
                }

                if (verbose_>2)  std::cout << Debug() << "... ... stub: " << istub << " rank: " << rank << " dist: " << dist << std::endl;

                vec_index_dist.push_back(std::make_pair(istub, std::make_pair(rank, dist)));

            } else {
                if (verbose_>2)  std::cout << Debug() << "... ... stub: " << istub << " fail cut!" << std::endl;
            }
        }

        // Sort by rank, then by smallest dist to largest
        // For future: also include delta_s?
        std::sort(vec_index_dist.begin(), vec_index_dist.end(), sortByUnsignedThenFloat);

        // Select only one stub per layer
        std::vector<unsigned> goodIndices(16, 999999);
        if (vec_index_dist.size()) {
            for (unsigned iistub=0; iistub<vec_index_dist.size(); ++iistub) {
                unsigned istub = vec_index_dist.at(iistub).first;
                float    dist  = vec_index_dist.at(iistub).second.second;

                unsigned moduleId = reader.vb_modId->at(istub);
                unsigned lay16    = compressLayer(decodeLayer(moduleId));

                // For each layer, takes the stub with min dist to simTrack
                if (goodIndices.at(lay16) == 999999 && dist < 26.0) {  // gets rid of stubs due to loopers
                    goodIndices.at(lay16) = istub;
                }
            }
        }

        //if (keep && goodIndices.at(0) == 999999)
        //    std::cout << Warning() << "... evt: " << ievt << " no stub in the first layer of the barrel!" << std::endl;
        //if (keep && goodIndices.at(6) != 999999 && goodIndices.at(11) != 999999)
        //    std::cout << Warning() << "... evt: " << ievt << " found stubs in the first layers of both positive and negative endcaps!" << std::endl;


        // _____________________________________________________________________
        // Now make keep-or-ignore decision per stub
        unsigned ngoodstubs = 0;
        for (unsigned istub=0; (istub<nstubs) && keep; ++istub) {
            bool keepstub = true;

            unsigned moduleId = reader.vb_modId->at(istub);

            // Check whether istub was an index stored for a good stub
            const unsigned count = std::count(goodIndices.begin(), goodIndices.end(), istub);
            if (!count)
                keepstub = false;

            if (verbose_>2)  std::cout << Debug() << "... ... stub: " << istub << " moduleId: " << moduleId << " keep? " << keepstub << std::endl;

            if (keepstub) {
                // Keep the stub and do something similar to insertion sort
                // First, find the position to insert (determined by moduleId)
                std::vector<unsigned>::const_iterator pos = std::upper_bound(reader.vb_modId->begin(), reader.vb_modId->begin()+ngoodstubs, moduleId);
                unsigned ipos = pos - reader.vb_modId->begin();

                // Insert, keeping only the 'ngoodstubs' elements
              //insertSorted(reader.vb_x->begin()         , ngoodstubs, ipos, reader.vb_x->at(istub));
              //insertSorted(reader.vb_y->begin()         , ngoodstubs, ipos, reader.vb_y->at(istub));
                insertSorted(reader.vb_z->begin()         , ngoodstubs, ipos, reader.vb_z->at(istub));
                insertSorted(reader.vb_r->begin()         , ngoodstubs, ipos, reader.vb_r->at(istub));
                insertSorted(reader.vb_eta->begin()       , ngoodstubs, ipos, reader.vb_eta->at(istub));
                insertSorted(reader.vb_phi->begin()       , ngoodstubs, ipos, reader.vb_phi->at(istub));
                insertSorted(reader.vb_coordx->begin()    , ngoodstubs, ipos, reader.vb_coordx->at(istub));
                insertSorted(reader.vb_coordy->begin()    , ngoodstubs, ipos, reader.vb_coordy->at(istub));
                insertSorted(reader.vb_trigBend->begin()  , ngoodstubs, ipos, reader.vb_trigBend->at(istub));
              //insertSorted(reader.vb_roughPt->begin()   , ngoodstubs, ipos, reader.vb_roughPt->at(istub));
              //insertSorted(reader.vb_clusWidth0->begin(), ngoodstubs, ipos, reader.vb_clusWidth0->at(istub));
              //insertSorted(reader.vb_clusWidth1->begin(), ngoodstubs, ipos, reader.vb_clusWidth1->at(istub));
                insertSorted(reader.vb_modId->begin()     , ngoodstubs, ipos, reader.vb_modId->at(istub));
                insertSorted(reader.vb_tpId->begin()      , ngoodstubs, ipos, reader.vb_tpId->at(istub));

                ++ngoodstubs;  // remember to increment
            }
        }
        assert(ngoodstubs <= nstubs);

        // _____________________________________________________________________
        // Now make keep-or-ignore decision per event

        // Check again min # of stubs
        require = (ngoodstubs >= MIN_NGOODSTUBS);
        if (!require)
            keep = false;

        if (keep)
            ++nKept;
        else  // do not keep any stub
            ngoodstubs = 0;

        if (keep && ngoodstubs > MAX_NGOODSTUBS) {
            std::cout << Warning() << "... evt: " << ievt << " simPt: " << simPt << " simEta: " << simEta << " simPhi: " << simPhi <<  " ngoodstubs: " << ngoodstubs << std::endl;
        }

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # good stubs: " << ngoodstubs << " keep? " << keep << std::endl;

      //reader.vb_x         ->resize(ngoodstubs);
      //reader.vb_y         ->resize(ngoodstubs);
        reader.vb_z         ->resize(ngoodstubs);
        reader.vb_r         ->resize(ngoodstubs);
        reader.vb_eta       ->resize(ngoodstubs);
        reader.vb_phi       ->resize(ngoodstubs);
        reader.vb_coordx    ->resize(ngoodstubs);
        reader.vb_coordy    ->resize(ngoodstubs);
        reader.vb_trigBend  ->resize(ngoodstubs);
      //reader.vb_roughPt   ->resize(ngoodstubs);
      //reader.vb_clusWidth0->resize(ngoodstubs);
      //reader.vb_clusWidth1->resize(ngoodstubs);
        reader.vb_modId     ->resize(ngoodstubs);
        reader.vb_tpId      ->resize(ngoodstubs);

        ++nRead;
        writer.fill();
    }

    if (nRead == 0) {
        std::cout << Error() << "Failed to read any event." << std::endl;
        return 1;
    }

    if (verbose_)  std::cout << Info() << Form("Read: %7ld, kept: %7ld", nRead, nKept) << std::endl;

    writer.write();

    return 0;
}


// _____________________________________________________________________________
// Main driver
int StubCleaner::run() {
    int exitcode = 0;
    Timing(1);

    exitcode = cleanStubs(po_.input, po_.output);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
