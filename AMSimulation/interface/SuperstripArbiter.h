#ifndef AMSimulation_SuperstripArbiter_h_
#define AMSimulation_SuperstripArbiter_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TriggerTowerMap.h"

#include <string>
#include <vector>
#include "TString.h"


namespace slhcl1tt {

enum SuperstripType {UNKNOWN, FIXEDWIDTH, PROJECTIVE, FOUNTAIN};

class SuperstripArbiter {
  public:
    // Constructor
    SuperstripArbiter();

    // Destructor
    ~SuperstripArbiter() {}

    // Operators
    unsigned superstripLocal(unsigned moduleId, float strip, float segment) const;
    unsigned superstripGlobal(unsigned moduleId, float r, float phi, float z, float ds) const;

    // Functions
    void setDefinition(TString definition, unsigned tt, const TriggerTowerMap* ttmap);

    unsigned nsuperstripsPerLayer() const { return nsuperstripsPerLayer_; }

    bool useGlobalCoord() const { return useGlobalCoord_; }

    // Debug
    void print();


  private:
    // Member functions
    unsigned compressModuleId(unsigned moduleId) const;

    unsigned superstripFixedwidth(unsigned moduleId, float strip, float segment) const;
    unsigned superstripProjective(unsigned moduleId, float r, float phi, float z, float ds) const;
    unsigned superstripFountain(unsigned moduleId, float r, float phi, float z, float ds) const;

    // Member data
    SuperstripType     sstype_;
    unsigned           nsuperstripsPerLayer_;
    bool               useGlobalCoord_;

    unsigned           fixedwidth_nstrips_;
    unsigned           fixedwidth_nz_;
    unsigned           fixedwidth_bit_lshift1_;
    unsigned           fixedwidth_bit_lshift2_;
    unsigned           fixedwidth_bit_rshift1_;
    unsigned           fixedwidth_bit_rshift2_;
    unsigned           projective_nx_;
    unsigned           projective_nz_;
    std::vector<float> projective_phiBins_;
    std::vector<float> projective_zBins_;
    float              fountain_sf_;
    unsigned           fountain_nz_;
    unsigned           fountain_max_nx_;
    std::vector<float> fountain_phiBins_;
    std::vector<float> fountain_zBins_;

    // Trigger tower geometry
    // modules_[i][j] is the module ID of the i-th layer, j-th module
    std::vector<std::vector<unsigned> > towerModules_;

    // phiMins_[i] is the phiMin boundary of the i-th layer
    // phiMaxs_[i] is the phiMax boundary of the i-th layer
    // zMins_[i] is the zMin boundary of the i-th layer
    // zMaxs_[i] is the zMax boundary of the i-th layer
    std::vector<float> phiMins_;
    std::vector<float> phiMaxs_;
    std::vector<float> zMins_;
    std::vector<float> zMaxs_;

    // phiWidths[i] is the phiWidth of the i-th layer
    std::vector<float> phiWidths_;
};

}  // namespace slhcl1tt

#endif
