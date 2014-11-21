#ifndef AMSimulationIO_CSVFileReader_h_
#define AMSimulationIO_CSVFileReader_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationIO/interface/Helper.h"

namespace slhcl1tt {


// _____________________________________________________________________________
class CSVFileReader {
  public:
    CSVFileReader(int verbose=1);
    ~CSVFileReader();

    int getTriggerTowerMap(TString src,
                           std::map<unsigned, std::vector<unsigned> >& ttmap);

    int getModuleCoordinateMap(TString src,
                               std::map<unsigned, ModuleCoordinate>& coordmap);

  protected:
    const int verbose_;
};

}  // namespace slhcl1tt

#endif

