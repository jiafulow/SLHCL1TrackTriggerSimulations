#ifndef AMSimulation_TriggerTowerMap_h_
#define AMSimulation_TriggerTowerMap_h_

#include <map>
#include <string>
#include <vector>
#include "TString.h"


namespace slhcl1tt {

// Define boundaries of each layer
struct LayerBounds {
    unsigned layer;
    float phiMin;
    float phiMax;
    float zMin;     // z for barrel, r for endcap
    float zMax;     // z for barrel, r for endcap

    void set(unsigned id, const std::vector<float>& values) {
        layer = id;
        phiMin = values.at(0);
        phiMax = values.at(1);
        zMin = values.at(2);
        zMax = values.at(3);
    }
};


// Define trigger towers
class TriggerTowerMap {
  public:
    // Constructor
    TriggerTowerMap();

    // Destructor
    ~TriggerTowerMap() {}

    // Functions
    // Read trigger tower map csv file
    void readTriggerTowerMap(TString csvfile);

    // Read trigger tower boundaries csv file
    void readTriggerTowerBoundaries(TString csvfile);

    // Get a reverse map for a particular tower
    // The map uses module ID as the key. Its value is a dummy.
    // (One could use std::set instead of std::map, but std::map is more recognizable)
    std::map<unsigned, bool> getTriggerTowerReverseMap(unsigned tt) const;

    // Get a vector of module IDs for a particular tower
    std::vector<unsigned> getTriggerTowerModules(unsigned tt) const;

    // Get a vector of LayerBounds=(layer,4 boundaries) for a particular tower
    std::vector<LayerBounds> getTriggerTowerBoundaries(unsigned tt) const;

    // Debug
    void print();

  private:
    // Mapping of {tower -> [modules]}
    std::map<unsigned, std::vector<unsigned> > ttmap_;

    // Mapping of {module -> [towers]}
    //std::map<unsigned, std::vector<unsigned> > ttrmap_;

    // Mapping of {tower -> [(layer, boundaries)]}
    std::map<unsigned, std::vector<LayerBounds> > ttboundaries_;
};

}  // namespace slhcl1tt

#endif
