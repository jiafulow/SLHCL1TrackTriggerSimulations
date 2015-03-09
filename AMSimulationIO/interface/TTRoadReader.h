#ifndef AMSimulationIO_TTRoadReader_h_
#define AMSimulationIO_TTRoadReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTRoad.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/BasicReader.h"


namespace slhcl1tt {

// _____________________________________________________________________________
class TTRoadReader : public BasicReader {
  public:
    TTRoadReader(int verbose=1);
    ~TTRoadReader();

    int init(TString src, TString prefix, TString suffix);

    // Roads
    std::vector<unsigned> *                             vr_patternRef;
    std::vector<unsigned> *                             vr_tower;
    std::vector<unsigned> *                             vr_nstubs;
    std::vector<std::vector<unsigned> > *               vr_superstripIds;
    std::vector<std::vector<std::vector<unsigned> > > * vr_stubRefs;
};


// _____________________________________________________________________________
class TTRoadWriter : public BasicWriter {
  public:
    TTRoadWriter(int verbose=1);
    ~TTRoadWriter();

    int init(TChain* tchain, TString out, TString prefix, TString suffix);

    void fill(const std::vector<TTRoad>& roads);

  protected:
    // Roads
    std::auto_ptr<std::vector<unsigned> >                             vr_patternRef;
    std::auto_ptr<std::vector<unsigned> >                             vr_tower;
    std::auto_ptr<std::vector<unsigned> >                             vr_nstubs;
    std::auto_ptr<std::vector<std::vector<unsigned> > >               vr_superstripIds;
    std::auto_ptr<std::vector<std::vector<std::vector<unsigned> > > > vr_stubRefs;
};

}  // namespace slhcl1tt

#endif
