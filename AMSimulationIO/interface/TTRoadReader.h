#ifndef AMSimulationIO_TTRoadReader_h_
#define AMSimulationIO_TTRoadReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTPattern.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/BasicReader.h"


namespace slhcl1tt {

// _____________________________________________________________________________
class TTRoadReader : public BasicReader {
  public:
    TTRoadReader(int verbose=1);
    ~TTRoadReader();

    int init(TString src, TString prefix, TString suffix);

    // Roads
    std::vector<id_type> *                  vr_bankIndex;
    std::vector<id_type> *                  vr_triggerTowerId;
    std::vector<count_type> *               vr_nsuperstrips;
    std::vector<std::vector<id_type> > *    vr_stubSuperstripIds;
    std::vector<std::vector<unsigned> > *   vr_stubRefs;
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
    std::auto_ptr<std::vector<id_type> >                  vr_bankIndex;
    std::auto_ptr<std::vector<id_type> >                  vr_triggerTowerId;
    std::auto_ptr<std::vector<count_type> >               vr_nsuperstrips;
    std::auto_ptr<std::vector<std::vector<id_type> > >    vr_stubSuperstripIds;
    std::auto_ptr<std::vector<std::vector<unsigned> > >   vr_stubRefs;
};

}  // namespace slhcl1tt

#endif
