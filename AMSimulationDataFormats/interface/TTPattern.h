#ifndef AMSimulationDataFormats_TTPattern_h_
#define AMSimulationDataFormats_TTPattern_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTSuperstrip.h"

#include <iosfwd>
#include <vector>


namespace slhcl1tt {

// TTPattern is a collection of 8 TTSuperstrips
// A POD type is used to reduce memory consumption.
struct TTPattern {
    count_type frequency;
    pattern_type id;
    pattern_bit_type bit;
};


// TTRoad is a collection of TTHit. In actuality, a road should be a word,
// and its relevant data is retrieved using a lookup table. In software,
// we simply skip this lookup.
// TTRoad is not a POD data type due to the use of std::vector<TTHit>.
class TTRoad {
  public:
    // Constructors
    TTRoad()
    : nSuperstrips_(0), bankIndex_(0), hits_() {}

    TTRoad(count_type nSuperstrips, id_type bankIndex, const std::vector<TTHit>& hits)
    : nSuperstrips_(nSuperstrips), bankIndex_(bankIndex), hits_(hits) {}

    TTRoad(const TTRoad& rhs)
    : nSuperstrips_(rhs.nSuperstrips_), bankIndex_(rhs.bankIndex_), hits_(rhs.hits_) {}

    // Destructor
    ~TTRoad() {}

    // Setters
    // none

    // Getters
    count_type nSuperstrips()       const { return nSuperstrips_; }

    id_type bankIndex()             const { return bankIndex_; }

    std::vector<TTHit> getHits()    const { return hits_; }
    TTHit getHit(int l)             const { return hits_.at(l); }


  private:
    count_type nSuperstrips_;
    id_type bankIndex_;
    std::vector<TTHit> hits_;
};


// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const pattern_type& patt);

std::ostream& operator<<(std::ostream& o, const TTPattern& patt);

std::ostream& operator<<(std::ostream& o, const std::vector<TTHit>& hits);

std::ostream& operator<<(std::ostream& o, const TTRoad& road);

}  // namespace slhcl1tt

#endif
