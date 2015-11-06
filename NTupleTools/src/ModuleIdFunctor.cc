#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/ModuleIdFunctor.h"

#include <cassert>

uint32_t ModuleIdFunctor::operator() (const DetId& id) const {
    uint32_t layer  = 999999;
    uint32_t ladder = 999999;
    uint32_t module = 999999;

    if (id.subdetId() == (int) PixelSubdetector::PixelBarrel) {
        PXBDetId pxbId(id);
        layer  = pxbId.layer();
        ladder = pxbId.ladder();
        module = pxbId.module();

    } else if (id.subdetId() == (int) PixelSubdetector::PixelEndcap) {
        PXFDetId pxfId(id);
        layer  = (pxfId.side() == 2) ? pxfId.disk() : pxfId.disk()+7;
        //ladder = pxfId.ring();
        ladder = pxfId.blade();
        module = pxfId.module();
    }

    assert(layer != 999999 && ladder != 999999 && module != 999999);
    return 10000*layer + 100*(ladder-1) + (module-1)/2;
}

