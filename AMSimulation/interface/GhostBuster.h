#ifndef AMSimulation_GhostBuster_h_
#define AMSimulation_GhostBuster_h_

#include <vector>

namespace slhcl1tt {

class GhostBuster {
  public:
    // Constructor
    GhostBuster() {}

    // Destructor
    ~GhostBuster() {}

    // Enum
    enum Flag { BAD=999999999 };

    // Is road 2 a ghost of road 1?
    bool isGhostRoad(const std::vector<unsigned>& superstripIds1, const std::vector<unsigned>& superstripIds2, const unsigned threshold=2) const;

    // Is track 2 a ghost of track 1?
    bool isGhostTrack(const std::vector<unsigned>& stubRefs1, const std::vector<unsigned>& stubRefs2, const unsigned threshold=2) const;

    // Debug
    void print();

  private:
    // Member data
    int verbose_;
};

}

#endif
