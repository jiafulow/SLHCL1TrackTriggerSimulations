#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitter.h"

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTTrackReader.h"


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
    if (writer.init(out, prefixTrack_, suffix_)) {
        std::cout << Error() << "Failed to initialize TTTrackWriter." << std::endl;
        return 1;
    }

    // _________________________________________________________________________
    // Loop over all events

    // Bookkeepers
    int nPassed = 0, nKept = 0;

    for (long long ievt=0; ievt<nEvents_; ++ievt) {
        if (reader.loadTree(ievt) < 0)  break;
        reader.getEntry(ievt);

        const unsigned nroads = reader.vr_hitRs->size();
        if (verbose_>1 && ievt%5000==0)  std::cout << Debug() << Form("... Processing event: %7lld, keeping: %7i, fitting: %7i", ievt, nKept, nPassed) << std::endl;
        if (verbose_>2)  std::cout << Debug() << "... evt: " << ievt << " # roads: " << nroads << std::endl;

//FIXME        if (!nroads) {  // skip if no road
            writer.fill(std::vector<TTTrack>());
            ++nKept;
            continue;
//FIXME        }

        std::vector<TTTrack> tracks;
        tracks.reserve(200);

        for (unsigned i=0; i<nroads; ++i) {
            const unsigned nhits = reader.vr_hitRs->at(i).size();
            //if (nhits<3) {
            //    std::cout << Error() << "Too few hits in a road: " << nhits << std::endl;
            //    return 1;
            //}

            std::vector<TTHit> hits;
            std::vector<ZR> hitsViewZR;
            std::vector<UV> hitsViewUV;
            for (unsigned j=0; j<nhits; ++j) {
                hits.emplace_back(TTHit{
                    reader.vr_hitRs->at(i).at(j),
                    reader.vr_hitPhis->at(i).at(j),
                    reader.vr_hitZs->at(i).at(j),
                    reader.vr_hitRErrors->at(i).at(j),
                    reader.vr_hitPhiErrors->at(i).at(j),
                    reader.vr_hitZErrors->at(i).at(j),
                    reader.vr_hitClusWidths->at(i).at(j),
                    reader.vr_hitStubWidths->at(i).at(j),
                    reader.vr_hitSuperstripIds->at(i).at(j),
                    reader.vr_hitTrkIds->at(i).at(j)
                });

                // In R-Z plane, a track is a straight line
                ZR hitViewZR(hits.back().r, hits.back().z);
                hitsViewZR.push_back(hitViewZR);

                // In X-Y plane, a track is a circle
                // Under conformal transfromation XY --> UV, a track becomes a straight line
                UV hitViewUV(hits.back().u(), hits.back().v());
                hitsViewUV.push_back(hitViewUV);
            }

            TTRoad road(reader.vr_nSuperstrips->at(i), reader.vr_bankIndex->at(i), hits);

            // Fitting starts here
            RetinaTrackFitterAlgo<ZR> retinaZR(po.pqType, po.pbins, po.qbins, po.pmin, po.qmin, po.pmax, po.qmax, po.sigma, po.minWeight);
            retinaZR.setHits(hitsViewZR);
            retinaZR.fillGrid();
            retinaZR.findMaxima();
            std::vector<TrackParam> params = retinaZR.getParams();

            if (verbose_>2) {
                std::cout << Debug() << "... ... road: " << i << " # params: " << params.size() << std::endl;
                for (unsigned j=0; j<params.size(); ++j)
                    std::cout << Debug() << "... ... ... p: " << params.at(j) << std::endl;
            }


            TTTrack track;  // FIXME
            if (!params.empty()) {
                TrackParam p = params.at(0);
                GlobalVector gv(
                    p.pt * std::cos(p.phi),
                    p.pt * std::sin(p.phi),
                    p.pt > 0 ? p.pt * std::sinh(p.eta) : 0
                );
                GlobalPoint gp(0., 0., p.dz);
                track.setMomentum(gv);
                track.setPOCA(gp);
                track.setChi2(p.chi2);
            }
            tracks.push_back(track);

            if ((int) tracks.size() >= maxTracks_)
                break;
        }

        if (! tracks.empty())
            ++nPassed;

        writer.fill(tracks);
        ++nKept;
    }

    if (verbose_)  std::cout << Info() << "Processed " << nEvents_ << " events, kept " << nKept << ", fitted " << nPassed << std::endl;

    long long nentries = writer.write();
    assert(nentries == nKept);

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
