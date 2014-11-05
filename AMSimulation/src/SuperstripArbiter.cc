#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/SuperstripArbiter.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"
using namespace slhcl1tt;

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <stdexcept>


void SuperstripArbiter::init() {
    if (barrel_subladder_sizes_.size() != barrel_n_)
        throw std::invalid_argument("Incorrect barrel subladder size is given.");
    if (barrel_submodule_sizes_.size() != barrel_n_)
        throw std::invalid_argument("Incorrect barrel submodule size is given.");
    if (endcap_subladder_sizes_.size() != endcap_n_)
        throw std::invalid_argument("Incorrect endcap subladder size is given.");
    if (endcap_submodule_sizes_.size() != endcap_n_)
        throw std::invalid_argument("Incorrect endcap submodule size is given.");

    // Only division by a power of two (2, 4, 8, 16, ...) is safe, as it corresponds to bit shifting
    // mostSigBit() rounds down to the nearest power of two
    const unsigned max_subladder_size = 5u;  // 2^5  = 32;
    const unsigned max_submodule_size = 10u; // 2^10 = 1024;

    for (unsigned i=0; i<barrel_n_; ++i) {
        // Convert to the number of bits to shift down to speed up integer division
        if (!(i<3))  // not PS modules
            barrel_subladder_sizes_.at(i) /= 16;
        barrel_subladder_sizes_.at(i) = mostSigBit(barrel_subladder_sizes_.at(i));
        barrel_submodule_sizes_.at(i) = mostSigBit(barrel_submodule_sizes_.at(i));

        if (barrel_subladder_sizes_.at(i) > max_subladder_size)
            throw std::invalid_argument("Incorrect barrel subladder size is given.");
        if (barrel_submodule_sizes_.at(i) > max_submodule_size)
            throw std::invalid_argument("Incorrect barrel submodule size is given.");
    }

    for (unsigned i=0; i<endcap_n_; ++i) {
        // Convert to the number of bits to shift down to speed up integer division
        if (!(i<9))  // not PS modules
            endcap_subladder_sizes_.at(i) /= 16;
        endcap_subladder_sizes_.at(i) = mostSigBit(endcap_subladder_sizes_.at(i));
        endcap_submodule_sizes_.at(i) = mostSigBit(endcap_submodule_sizes_.at(i));

        if (endcap_subladder_sizes_.at(i) > max_subladder_size)
            throw std::invalid_argument("Incorrect endcap subladder size is given.");
        if (endcap_submodule_sizes_.at(i) > max_submodule_size)
            throw std::invalid_argument("Incorrect endcap submodule size is given.");
    }

    // Find min superstrip size
    unsigned barrel_subladder_bits = max_subladder_size - *(std::min_element(barrel_subladder_sizes_.begin(), barrel_subladder_sizes_.begin() + 3) );  // only PS modules
    unsigned barrel_submodule_bits = max_submodule_size - *(std::min_element(barrel_submodule_sizes_.begin(), barrel_submodule_sizes_.end()) );

    unsigned endcap_subladder_bits = max_subladder_size - *(std::min_element(endcap_subladder_sizes_.begin(), endcap_subladder_sizes_.begin() + 9) );  // only PS modules
    unsigned endcap_submodule_bits = max_submodule_size - *(std::min_element(endcap_submodule_sizes_.begin(), endcap_submodule_sizes_.end()) );

    // Take the max in barrel and endcap, assign to barrel
    max_subladder_bits_ = std::max(barrel_subladder_bits, endcap_subladder_bits);
    max_submodule_bits_ = std::max(barrel_submodule_bits, endcap_submodule_bits);
    max_subladder_bits_ += max_submodule_bits_;  // include submodule bits

    // _________________________________________________________________________
    // Tracker geomerty info
    // Set divisions
    barrel_z_divisions_ = {  // 6 elements
        63, 55, 54, 24, 24, 24
    };

    barrel_phi_divisions_ = {  // 6 elements
        16, 24, 34, 48, 62, 76
    };

    endcap_ring_divisions_ = {  // 15 elements
        20, 24, 28, 28, 32, 36, 36, 40, 40, 52, 56, 64, 68, 76, 80
    };

    // Set offsets
    barrel_layer_offsets_.resize(barrel_n_+1);
    barrel_layer_offsets_[0] = 0;
    for (unsigned i=0; i<barrel_n_; ++i) {
        barrel_layer_offsets_[i+1] = barrel_layer_offsets_[i] + barrel_phi_divisions_[i] * barrel_z_divisions_[i];
    }

    endcap_ring_offsets_.resize(endcap_n_+1);
    endcap_ring_offsets_[0] = 0;
    for (unsigned i=0; i<endcap_n_; ++i) {
        endcap_ring_offsets_[i+1] = endcap_ring_offsets_[i] + endcap_ring_divisions_[i];
    }

    // Virtual geometry
    unsigned calo_divisions = 1;  // dummy
    calo_offsets_.resize(2);
    calo_offsets_[0] = barrel_layer_offsets_.at(barrel_n_) + 10 * endcap_ring_offsets_.at(endcap_n_);
    calo_offsets_[1] = calo_offsets_[0] + calo_divisions;

    unsigned muon_divisions = 1;  // dummy
    muon_offsets_.resize(2);
    muon_offsets_[0] = calo_offsets_[1];
    muon_offsets_[1] = muon_offsets_[0] + muon_divisions;

    fake_offsets_.resize(1);
    fake_offsets_[0] = muon_offsets_[1];
}

// Use 4-Byte integers everywhere as it's more native to machines
unsigned SuperstripArbiter::superstrip(unsigned lay, unsigned lad, unsigned mod,
                                       unsigned col, unsigned row,
                                       const bool isHalfStrip) const {
    unsigned h = 0;
    lay = compressLayer(lay);  // transform lay

    if (isHalfStrip) {  // transform col and row
        col >>= 1;
        row >>= 1;
    }

    if (lay < 6) {          // in barrel
        h += barrel_layer_offsets_.at(lay);
        h += mod * barrel_phi_divisions_.at(lay);
        h += lad;                                 // lad is the local phi coord

        col >>= barrel_subladder_sizes_.at(lay);
        row >>= barrel_submodule_sizes_.at(lay);

        h = (h << max_subladder_bits_) | (col << max_submodule_bits_) | row;

    } else if (lay < 16) {  // in endcap
        lay -= 6;
        h += barrel_layer_offsets_.at(6);        // after all barrel layers
        h += lay * endcap_ring_offsets_.at(15);  // after all previous endcap rings
        h += endcap_ring_offsets_.at(lad);
        h += mod;                                // mod is the local phi coord

        col >>= endcap_subladder_sizes_.at(lad);
        row >>= endcap_submodule_sizes_.at(lad);

        h = (h << max_subladder_bits_) | (col << max_submodule_bits_) | row;

    } else if (lay == 16) { // virtual calo layer
        h += calo_offsets_.at(0);
        h = (h << max_subladder_bits_);

    } else if (lay == 17) { // virtual muon layer
        h += muon_offsets_.at(0);
        h = (h << max_subladder_bits_);

    } else if (lay == 18) { // fake superstrip
        h += fake_offsets_.at(0);
        h = (h << max_subladder_bits_);

    } else {
        h = 0xffffffff;
        throw std::invalid_argument("Incorrect layer");
    }

    return h;
}

unsigned SuperstripArbiter::module(unsigned lay, unsigned lad, unsigned mod) const {
    unsigned h = 0;
    lay = compressLayer(lay);  // transform lay

    if (lay < 6) {          // in barrel
        h += barrel_layer_offsets_.at(lay);
        h += mod * barrel_phi_divisions_.at(lay);
        h += lad;                                 // lad is the local phi coord

    } else if (lay < 16) {  // in endcap
        lay -= 6;
        h += barrel_layer_offsets_.at(6);        // after all barrel layers
        h += lay * endcap_ring_offsets_.at(15);  // after all previous endcap rings
        h += endcap_ring_offsets_.at(lad);
        h += mod;                                // mod is the local phi coord

    } else {
        h = 0xffffffff;
        throw std::invalid_argument("Incorrect layer");
    }

    return h;
}

unsigned SuperstripArbiter::superstrip_luciano(unsigned lay, float phi, float z,
                                               const float unit_phi, const float unit_z) const {
    unsigned h = 0;
    lay = compressLayer(lay);  // transform lay

    const unsigned n_phi = M_PI*2. / unit_phi + 1e-3;
    const unsigned n_z = 360*2. / unit_z + 1e-3;

    if (lay < 16) {
        phi += M_PI;  // -M_PI is the lowest phi value
        z += 360.;    // -360. is the lowest z

        phi = (phi < 0.) ? 0. : (phi >= M_PI*2. ? M_PI*2. - 1e-3 : phi);
        z = (z < 0.) ? 0. : (z >= 360.*2. ? 360.*2. - 1e-3 : z);

        h = (lay * n_z * n_phi) + unsigned(z / unit_z) * n_phi + unsigned(phi / unit_phi);

    } else if (lay < 18) {
        h = lay * n_z * n_phi;

    } else {
        h = 18 * n_z * n_phi;

    }

    return h;
}

void SuperstripArbiter::print() {
    std::cout << "barrel subladder size: ";
    std::copy(barrel_subladder_sizes_.begin(), barrel_subladder_sizes_.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "barrel submodule size: ";
    std::copy(barrel_submodule_sizes_.begin(), barrel_submodule_sizes_.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "endcap subladder size: ";
    std::copy(endcap_subladder_sizes_.begin(), endcap_subladder_sizes_.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "endcap submodule size: ";
    std::copy(endcap_submodule_sizes_.begin(), endcap_submodule_sizes_.end(), std::ostream_iterator<unsigned>(std::cout, " "));
    std::cout << std::endl;
    std::cout << "subladder bits: " << max_subladder_bits_ << " submodule bits: " << max_submodule_bits_ << std::endl;
}
