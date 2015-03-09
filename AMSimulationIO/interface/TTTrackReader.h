#ifndef AMSimulationIO_TTTrackReader_h_
#define AMSimulationIO_TTTrackReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack2.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/TTRoadReader.h"


namespace slhcl1tt {

// _____________________________________________________________________________
class TTTrackReader : public TTRoadReader {
  public:
    TTTrackReader(int verbose=1);
    ~TTTrackReader();

    int init(TString src, TString prefixRoad, TString prefixTrack, TString suffix);

    // Tracks
    // FIXME
};


// _____________________________________________________________________________
class TTTrackWriter : public BasicWriter {
  public:
    TTTrackWriter(int verbose=1);
    ~TTTrackWriter();

    int init(TChain* tchain, TString out, TString prefix, TString suffix);

    void fill(const std::vector<TTTrack>& tracks);

    void fill(const std::vector<TTTrack2>& tracks);

  protected:
    // Tracks
    std::auto_ptr<std::vector<float> >                  vt_px;
    std::auto_ptr<std::vector<float> >                  vt_py;
    std::auto_ptr<std::vector<float> >                  vt_pz;
    std::auto_ptr<std::vector<float> >                  vt_pt;
    std::auto_ptr<std::vector<float> >                  vt_eta;
    std::auto_ptr<std::vector<float> >                  vt_phi;
    std::auto_ptr<std::vector<float> >                  vt_vx;
    std::auto_ptr<std::vector<float> >                  vt_vy;
    std::auto_ptr<std::vector<float> >                  vt_vz;
    std::auto_ptr<std::vector<float> >                  vt_rinv;
    std::auto_ptr<std::vector<float> >                  vt_phi0;
    std::auto_ptr<std::vector<float> >                  vt_cottheta;
    std::auto_ptr<std::vector<float> >                  vt_z0;
    std::auto_ptr<std::vector<float> >                  vt_d0;
    std::auto_ptr<std::vector<float> >                  vt_chi2;
    std::auto_ptr<std::vector<int> >                    vt_ndof;
    std::auto_ptr<std::vector<float> >                  vt_chi2_phi;
    std::auto_ptr<std::vector<float> >                  vt_chi2_z;
    std::auto_ptr<std::vector<int> >                    vt_tpId;
    std::auto_ptr<std::vector<unsigned> >               vt_tower;
    std::auto_ptr<std::vector<unsigned> >               vt_roadRef;
    std::auto_ptr<std::vector<std::vector<unsigned> > > vt_stubRefs;
};

}  // namespace slhcl1tt

#endif
