#ifndef NTupleTools_ModuleIdFunctor_h_
#define NTupleTools_ModuleIdFunctor_h_

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/SiPixelDetId/interface/PXBDetId.h"
#include "DataFormats/SiPixelDetId/interface/PXFDetId.h"

class ModuleIdFunctor {
  public:
    uint32_t operator() (const DetId& id) const;
};

#endif

