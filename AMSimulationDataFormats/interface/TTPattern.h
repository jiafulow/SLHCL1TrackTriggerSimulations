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


// TTRoad contains info of a pattern that has fired. It consists of the index
// of the pattern in bank, its number of superstrips, and the indices
// of the stubs in those superstrips. The stub indices are serialized,
// each accompanied by a superstrip id. One or more stubs can be in any
// superstrip.
// Note that nsuperstrips is NOT the size of stubSuperstripIds vector.
// nsuperstrips is the number of superstrips in the pattern. The size of
// stubSuperstripIds vector is the number of stubs in all superstrips in
// the road.
// TTRoad is not a POD type due to the use of std::vector.
class TTRoad {
  public:
    // Constructors
    TTRoad()
    : bankIndex_(0), triggerTowerId_(0), nsuperstrips_(0),
      stubSuperstripIds_(), stubRefs_() {}

    TTRoad(id_type bankIndex, id_type triggerTowerId, count_type nsuperstrips,
           const std::vector<id_type>& stubSuperstripIds, const std::vector<unsigned>& stubRefs)
    : bankIndex_(bankIndex), triggerTowerId_(triggerTowerId), nsuperstrips_(nsuperstrips),
      stubSuperstripIds_(stubSuperstripIds), stubRefs_(stubRefs) {}

    TTRoad(const TTRoad& rhs)
    : bankIndex_(rhs.bankIndex_), triggerTowerId_(rhs.triggerTowerId_), nsuperstrips_(rhs.nsuperstrips_),
      stubSuperstripIds_(rhs.stubSuperstripIds_), stubRefs_(rhs.stubRefs_) {}

    // Destructor
    ~TTRoad() {}

    // Setters
    // none

    // Getters
    id_type bankIndex()                         const { return bankIndex_; }

    id_type triggerTowerId()                    const { return triggerTowerId_; }

    count_type nsuperstrips()                   const { return nsuperstrips_; }

    std::vector<id_type> stubSuperstripIds()    const { return stubSuperstripIds_; }
    id_type stubSuperstripId(int l)             const { return stubSuperstripIds_.at(l); }

    std::vector<unsigned> stubRefs()            const { return stubRefs_; }
    unsigned stubRef(int l)                     const { return stubRefs_.at(l); }


  private:
    id_type bankIndex_;
    id_type triggerTowerId_;
    count_type nsuperstrips_;
    std::vector<id_type>  stubSuperstripIds_;
    std::vector<unsigned> stubRefs_;
};


// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const pattern_type& patt);

std::ostream& operator<<(std::ostream& o, const TTPattern& patt);

std::ostream& operator<<(std::ostream& o, const TTRoad& road);

}  // namespace slhcl1tt

#endif
