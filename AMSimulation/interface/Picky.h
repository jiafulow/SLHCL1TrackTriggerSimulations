#ifndef AMSimulation_Picky_h_
#define AMSimulation_Picky_h_

#include <vector>


namespace slhcl1tt {

class Picky {
  public:
    // Constructor
    Picky();

    // Destructor
    ~Picky() {}

    // Functions
    // Apply cuts on dphi, dr and dz, depending on layer (compressed to 0-15)
    // The absolute values of dphi, dr and dz are used.
    bool applyCuts(unsigned lay16, const float dphi, const float dr, const float dz);

    // Decide the rank based on r (in barrel) or z (in endcap), depending on layer (compressed to 0-15)
    unsigned findRank(unsigned lay16, const float r, const float z);

    // Debug
    void print();

  private:
    // Member data
    std::vector<float>  barrel_r_coords_;
    std::vector<float>  barrel_phi_cuts_;
    std::vector<float>  barrel_z_cuts_;

    std::vector<float>  endcap_z_coords_;
    std::vector<float>  endcap_phi_cuts_;
    std::vector<float>  endcap_r_cuts_;
};

}

#endif
