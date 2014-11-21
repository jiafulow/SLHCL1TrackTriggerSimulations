#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/StubCleaner.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTStubReader.h"

static const unsigned MIN_NGOODSTUBS = 3;
static const unsigned MAX_NGOODSTUBS = 8;

namespace {
bool sortByFloat(const std::pair<unsigned, float>& lhs, const std::pair<unsigned, float>& rhs) {
    return lhs.second < rhs.second;
}

template<typename RandomAccessIterator, typename Size, typename T>
void insertSorted(RandomAccessIterator first, Size pos, Size len, const T value) {
    first += len;
    len -= pos;
    while (len>0) {
        *first = std::move(*(first-1));
        --len;
        --first;
    }
    *first = std::move(value);
}
}


// _____________________________________________________________________________
// Select one stub per layer, one hit per stub.
// If an event fails selection, all stubs are removed
int StubCleaner::cleanStubs(TString src, TString out) {
    if (verbose_)  std::cout << Info() << "Reading " << nEvents_ << " events and cleaning them." << std::endl;

    // _________________________________________________________________________
    // For reading events
    TTStubReader reader(verbose_);
    if (reader.init(src, false)) {
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
      //tchain->SetBranchStatus("TTStubs_x"         , 1);  // sync with BasicReader::init()
      //tchain->SetBranchStatus("TTStubs_y"         , 1);  // sync with BasicReader::init()
        tchain->SetBranchStatus("TTStubs_z"         , 1);
        tchain->SetBranchStatus("TTStubs_r"         , 1);
        tchain->SetBranchStatus("TTStubs_eta"       , 1);
        tchain->SetBranchStatus("TTStubs_phi"       , 1);
        tchain->SetBranchStatus("TTStubs_coordx"    , 1);
        tchain->SetBranchStatus("TTStubs_coordy"    , 1);
      //tchain->SetBranchStatus("TTStubs_roughPt"   , 1);  // sync with BasicReader::init()
        tchain->SetBranchStatus("TTStubs_trigBend"  , 1);
      //tchain->SetBranchStatus("TTStubs_clusWidth" , 1);  // sync with BasicReader::init()
        tchain->SetBranchStatus("TTStubs_modId"     , 1);
        tchain->SetBranchStatus("TTStubs_tpId"      , 1);
    }

    // For event selection
    TTreeFormula* ttf_event = reader.addFormula(eventSelect_);

    // For writing
    TTStubWriter writer(verbose_);
    if (writer.init(reader.getChain(), out)) {
        std::cout << Error() << "Failed to initialize TTStubWriter." << std::endl;
        return 1;
    }


    // _________________________________________________________________________
    // Loop over all events

    const int good_tpId = 0, unmatch_tpId = -1;

    // Bookkeepers
    int nPassed = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nstubs = reader.vb_modId->size();
        if (verbose_>1 && ievt%50000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, passing: %7i", ievt, nKept, nPassed) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # stubs: " << nstubs << std::endl;

        if (!nstubs) {  // skip if no stub
            ++nKept;
            writer.fill();
            continue;
        }

        if (nstubs > 100000) {
            std::cout << Error() << "Way too many stubs: " << nstubs << std::endl;
            return 1;
        }

        // _____________________________________________________________________
        // Start cleaning
        bool keep = true;

        // Check event selection
        int ndata = ttf_event->GetNdata();
        if (ndata && filter_)
            keep = (ttf_event->EvalInstance() != 0);

        // Check min # of layers
        bool require = (nstubs >= MIN_NGOODSTUBS);
        if (!require && filter_)
            keep = false;

        // Check sim info
        assert(reader.vp_pt->size() == 1);
        float simPt    = reader.vp_pt->front();
        float simEta   = reader.vp_eta->front();
        float simPhi   = reader.vp_phi->front();
        float simVx    = reader.vp_vx->front();
        float simVy    = reader.vp_vy->front();
        float simVz    = reader.vp_vz->front();
        int simCharge  = reader.vp_charge->front();

        // Apply pt, eta, phi requirements
        bool sim = (po.minPt  <= simPt  && simPt  <= po.maxPt  &&
                    po.minEta <= simEta && simEta <= po.maxEta &&
                    po.minPhi <= simPhi && simPhi <= po.maxPhi);
        if (!sim && filter_)
            keep = false;

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " simPt: " << simPt << " simEta: " << simEta << " simPhi: " << simPhi << " keep? " << keep << std::endl;

        // _____________________________________________________________________
        // Remove multiple stubs in one layer
        std::vector<std::pair<unsigned, float> > vec_index_dist;
        for (unsigned l=0; (l<nstubs) && keep; ++l) {
            int tpId = reader.vb_tpId->at(l);  // check sim info
            if (tpId == good_tpId || tpId == unmatch_tpId) {  // also catch stubs that fail to find a matched simTrack
                //float stub_eta = reader.vb_eta->at(l);
                //float dEta = std::abs(simEta - stub_eta);
                //if (dEta > 0.8)  // way too far
                //    continue;

                float bendPhi = simPhi - 0.3*3.8*reader.vb_r->at(l)*1e-2 * (simCharge/simPt) / 2.0;
                float dX = simVx + reader.vb_r->at(l) * (std::cos(bendPhi) - std::cos(reader.vb_phi->at(l)));
                float dY = simVy + reader.vb_r->at(l) * (std::sin(bendPhi) - std::sin(reader.vb_phi->at(l)));
                float dZ = simVz + reader.vb_r->at(l) * std::sinh(simEta) - reader.vb_z->at(l);  // cot(theta) = sinh(eta)
                float dR = std::sqrt(dX*dX + dY*dY + dZ*dZ);
                vec_index_dist.push_back(std::make_pair(l, dR));
            }
        }

        // Sort: smallest dR to largest
        std::sort(vec_index_dist.begin(), vec_index_dist.end(), sortByFloat);

        // Select only one stub per layer
        std::vector<unsigned> goodLayerStubs(16, 999999);
        if (vec_index_dist.size()) {
            id_type moduleId, lay, lay16;
            for (unsigned ll=0; (ll<vec_index_dist.size()) && keep; ++ll) {
                unsigned l = vec_index_dist.at(ll).first;
                float dist = vec_index_dist.at(ll).second;

                moduleId = reader.vb_modId->at(l);
                lay      = decodeLayer(moduleId);
                lay16    = compressLayer(lay);
                assert(lay16 < 16);

                // For each layer, takes the stub with min dR to simTrack
                if (goodLayerStubs.at(lay16) == 999999 && dist < 26.0) {  // CUIDADO: gets rid of stubs due to loopers
                    goodLayerStubs.at(lay16) = l;
                }
            }
        }

        //if (keep && goodLayerStubs.at(0) == 999999)
        //    std::cout << Warning() << "... evt: " << ievt << " no stub in the first layer of the barrel!" << std::endl;
        //if (keep && goodLayerStubs.at(6) != 999999 && goodLayerStubs.at(11) != 999999)
        //    std::cout << Warning() << "... evt: " << ievt << " found stubs in the first layers of both positive and negative endcaps!" << std::endl;


        // _____________________________________________________________________
        // Now make keep-or-ignore decision per stub
        unsigned ngoodstubs = 0;
        id_type moduleId;
        for (unsigned l=0; (l<nstubs) && keep; ++l) {
            bool keepstub = true;

            moduleId = reader.vb_modId->at(l);
            if (verbose_>2)  std::cout << Debug() << "... ... stub: " << l << " moduleId: " << moduleId << " tpId: " << reader.vb_tpId->at(l) << std::endl;

            // Check whether the index l was stored as a good stub
            const unsigned& count = std::count(goodLayerStubs.begin(), goodLayerStubs.end(), l);
            if (!count)  // do not keep even if filter_ is false
                keepstub = false;

            if (keepstub) {
                // Keep and do something similar to insertion sort
                // First, find the correct position to insert (according to moduleId)
                std::vector<unsigned>::const_iterator pos = reader.vb_modId->begin()+ngoodstubs;
                unsigned ipos = ngoodstubs;
                while (ipos != 0 && moduleId < *(--pos))  // start comparing from the tail
                    --ipos;

                // Insert, keeping only the 'ngoodstubs' elements
              //insertSorted(reader.vb_x->begin()        , ipos, ngoodstubs, reader.vb_x->at(l));
              //insertSorted(reader.vb_y->begin()        , ipos, ngoodstubs, reader.vb_y->at(l));
                insertSorted(reader.vb_z->begin()        , ipos, ngoodstubs, reader.vb_z->at(l));
                insertSorted(reader.vb_r->begin()        , ipos, ngoodstubs, reader.vb_r->at(l));
                insertSorted(reader.vb_eta->begin()      , ipos, ngoodstubs, reader.vb_eta->at(l));
                insertSorted(reader.vb_phi->begin()      , ipos, ngoodstubs, reader.vb_phi->at(l));
                insertSorted(reader.vb_coordx->begin()   , ipos, ngoodstubs, reader.vb_coordx->at(l));
                insertSorted(reader.vb_coordy->begin()   , ipos, ngoodstubs, reader.vb_coordy->at(l));
              //insertSorted(reader.vb_roughPt->begin()  , ipos, ngoodstubs, reader.vb_roughPt->at(l));
                insertSorted(reader.vb_trigBend->begin() , ipos, ngoodstubs, reader.vb_trigBend->at(l));
                insertSorted(reader.vb_modId->begin()    , ipos, ngoodstubs, reader.vb_modId->at(l));
                insertSorted(reader.vb_tpId->begin()     , ipos, ngoodstubs, reader.vb_tpId->at(l));

                ++ngoodstubs;  // remember to increment
            }
            if (verbose_>2)  std::cout << Debug() << "... ... stub: " << l << " keep? " << keepstub << std::endl;
        }
        assert(ngoodstubs <= nstubs);

        // _____________________________________________________________________
        // Now make keep-or-ignore decision per event

        // Check again min # of layers
        require = (ngoodstubs >= MIN_NGOODSTUBS);
        if (!require && filter_)
            keep = false;

        if (keep)
            ++nPassed;
        else //  do not keep any stub
            ngoodstubs = 0;

        if (keep && ngoodstubs > MAX_NGOODSTUBS) {
            std::cout << Warning() << "... evt: " << ievt << " simPt: " << simPt << " simEta: " << simEta << " simPhi: " << simPhi <<  " ngoodstubs: " << ngoodstubs << std::endl;
        }

        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # good stubs: " << ngoodstubs << " keep? " << keep << std::endl;

      //reader.vb_x        ->resize(ngoodstubs);
      //reader.vb_y        ->resize(ngoodstubs);
        reader.vb_z        ->resize(ngoodstubs);
        reader.vb_r        ->resize(ngoodstubs);
        reader.vb_eta      ->resize(ngoodstubs);
        reader.vb_phi      ->resize(ngoodstubs);
        reader.vb_coordx   ->resize(ngoodstubs);
        reader.vb_coordy   ->resize(ngoodstubs);
      //reader.vb_roughPt  ->resize(ngoodstubs);
        reader.vb_trigBend ->resize(ngoodstubs);
        reader.vb_modId    ->resize(ngoodstubs);
        reader.vb_tpId     ->resize(ngoodstubs);

        ++nKept;
        writer.fill();
    }
    if (verbose_)  std::cout << Info() << "Processed and kept " << nKept << " events, passed " << nPassed << std::endl;

    long long nentries = writer.writeTree();
    assert(nentries == nKept);

    return 0;
}


// _____________________________________________________________________________
// Main driver
int StubCleaner::run(TString src, TString out) {
    int exitcode = 0;
    Timing(1);

    exitcode = cleanStubs(src, out);
    if (exitcode)  return exitcode;
    Timing();

    return exitcode;
}
