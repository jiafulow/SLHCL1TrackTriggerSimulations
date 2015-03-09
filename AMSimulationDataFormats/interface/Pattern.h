#ifndef AMSimulationDataFormats_Pattern_h_
#define AMSimulationDataFormats_Pattern_h_

#include <stdint.h>
#include <array>
#include <iosfwd>

namespace slhcl1tt {

typedef uint32_t superstrip_type;
typedef uint16_t superstrip_bit_type;
typedef uint16_t frequency_type;

typedef std::array<superstrip_type,8> pattern_type;
typedef std::array<superstrip_bit_type,8> pattern_bit_type;


// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const pattern_type& patt);

std::ostream& operator<<(std::ostream& o, const pattern_bit_type& patt);

}  // namespace slhcl1tt

#endif
