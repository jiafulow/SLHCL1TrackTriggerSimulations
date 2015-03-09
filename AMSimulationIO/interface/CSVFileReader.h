#ifndef AMSimulationIO_CSVFileReader_h_
#define AMSimulationIO_CSVFileReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"

namespace slhcl1tt {


// _____________________________________________________________________________
class CSVFileReader {
  public:
    CSVFileReader(int verbose=1);
    ~CSVFileReader();

    int getTriggerTowerMap(TString src, unsigned tt,
                           std::map<unsigned, std::vector<unsigned> >& ttmap,
                           std::map<unsigned, std::vector<unsigned> >& ttrmap);

  protected:
    const int verbose_;
};

}  // namespace slhcl1tt

#endif

