#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Picky.h"
using namespace slhcl1tt;

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <stdexcept>

static const unsigned NLAYERS = 6;
static const unsigned NDISKS = 5;


// _____________________________________________________________________________
Picky::Picky() {

    // 4 possible r coordinates per barrel layer
    barrel_r_coords_ = {
        21.3678,21.9722,23.2104,23.8148,
        34.1546,34.7590,36.1954,36.7998,
        49.2178,49.8222,51.2582,51.8626,
        67.0078,67.6122,69.0080,69.6124,
        87.1979,87.8023,89.1981,89.8025,
        106.4078,107.0122,108.4078,109.0122
    };

    // 4 possible z coordinates per endcap disk
    endcap_z_coords_ = {
        128.494,130.074,132.614,134.194,
        153.295,154.875,157.415,158.995,
        182.654,184.234,186.774,188.354,
        217.408,218.988,221.528,223.108,
        258.550,260.130,262.670,264.250
    };

    // phi cut per barrel layer
    //barrel_phi_cuts_ = { 0.004, 0.004, 0.005, 0.007, 0.009, 0.012 };
    barrel_phi_cuts_ = { 0.100, 0.100, 0.100, 0.100, 0.100, 0.100 };  // disabled

    // z cut per barrel layer
    //barrel_z_cuts_   = { 0.300, 0.400, 0.500, 3.000, 3.500, 4.000 };
    barrel_z_cuts_   = { 9.900, 9.900, 9.900, 9.900, 9.900, 9.900 };  // disabled

    // phi cut per endcap disk
    endcap_phi_cuts_ = { 0.100, 0.100, 0.100, 0.100, 0.100 };  // disabled

    // r cut per endcap disk
    endcap_r_cuts_   = { 9.900, 9.900, 9.900, 9.900, 9.900 };  // disabled

    assert(barrel_r_coords_.size() == NLAYERS * 4 && barrel_phi_cuts_.size() == NLAYERS && barrel_z_cuts_.size() == NLAYERS);
    assert(endcap_z_coords_.size() == NDISKS * 4 && endcap_phi_cuts_.size() == NDISKS && endcap_r_cuts_.size() == NDISKS);
}

// _____________________________________________________________________________
bool Picky::applyCuts(unsigned lay16, const float dphi, const float dr, const float dz) {
    if (lay16 < 6) {
        return (std::abs(dphi) < barrel_phi_cuts_.at(lay16) && std::abs(dz) < barrel_z_cuts_.at(lay16));

    } else if (lay16 < 11) {
        lay16 -= 6;
        return (std::abs(dphi) < endcap_phi_cuts_.at(lay16) && std::abs(dr) < endcap_r_cuts_.at(lay16));

    } else if (lay16 < 16) {
        lay16 -= 11;
        return (std::abs(dphi) < endcap_phi_cuts_.at(lay16) && std::abs(dr) < endcap_r_cuts_.at(lay16));

    }
    return true;
}

// _____________________________________________________________________________
unsigned Picky::findRank(unsigned lay16, const float r, const float z) {
    static const float eps = 5e-3;

    unsigned j=3;
    if (lay16 < 6) {
        for (unsigned i=0; i<4; ++i, --j)
            if (r > (barrel_r_coords_.at(lay16*4 + j) - eps))
                break;

    } else if (lay16 < 11) {
        lay16 -= 6;
        for (unsigned i=0; i<4; ++i, --j)
            if (std::abs(z) > (endcap_z_coords_.at(lay16*4 + j) - eps))
                break;

    } else if (lay16 < 16) {
        lay16 -= 11;
        for (unsigned i=0; i<4; ++i, --j)
            if (std::abs(z) > (endcap_z_coords_.at(lay16*4 + j) - eps))
                break;
    }
    assert(j <= 3);

    // j=3 has larger radius than j=2,1,0
    // j=1,2 are closer to average radius than j=3,4
    // For future: in endcap, stubs in PS module should rank higher than stubs in 2S module
    static const float ranks[4] = {2,0,1,3};
    return ranks[j];
}

// _____________________________________________________________________________
void Picky::print() {
    std::cout << "barrel r: ";
    std::copy(barrel_r_coords_.begin(), barrel_r_coords_.end(), std::ostream_iterator<float>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "barrel phi cuts: ";
    std::copy(barrel_phi_cuts_.begin(), barrel_phi_cuts_.end(), std::ostream_iterator<float>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "barrel z cuts: ";
    std::copy(barrel_z_cuts_.begin(), barrel_z_cuts_.end(), std::ostream_iterator<float>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "endcap z: ";
    std::copy(endcap_z_coords_.begin(), endcap_z_coords_.end(), std::ostream_iterator<float>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "endcap phi cuts: ";
    std::copy(endcap_phi_cuts_.begin(), endcap_phi_cuts_.end(), std::ostream_iterator<float>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "endcap r cuts: ";
    std::copy(endcap_r_cuts_.begin(), endcap_r_cuts_.end(), std::ostream_iterator<float>(std::cout, " "));
    std::cout << std::endl;
}
