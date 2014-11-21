#ifndef AMSimulation_SuperstripArbiter_h_
#define AMSimulation_SuperstripArbiter_h_

#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace slhcl1tt {

class SuperstripArbiter {
  public:
    // Constructors
    // subladder, submodule sizes are expressed as if each module has
    // 32 pixels and 1024 strips, regardless of PS or 2S modules
    SuperstripArbiter(unsigned subladder, unsigned submodule)
    : barrel_n_(6), endcap_n_(15),
      barrel_subladder_sizes_(barrel_n_, subladder), barrel_submodule_sizes_(barrel_n_, submodule),
      endcap_subladder_sizes_(endcap_n_, subladder), endcap_submodule_sizes_(endcap_n_, submodule) {
        init();
    }

    SuperstripArbiter(const std::vector<unsigned>& b_subladders, const std::vector<unsigned>& b_submodules,
                      const std::vector<unsigned>& e_subladders, const std::vector<unsigned>& e_submodules)
    : barrel_n_(6), endcap_n_(15),
      barrel_subladder_sizes_(b_subladders), barrel_submodule_sizes_(b_submodules),
      endcap_subladder_sizes_(e_subladders), endcap_submodule_sizes_(e_submodules) {
        init();
    }

    // Destructor
    ~SuperstripArbiter() {}


    // Operators
    // Return the superstrip address given the strip address
    unsigned superstrip(unsigned lay, unsigned lad, unsigned mod,
                        unsigned col, unsigned row,
                        const bool isHalfStrip=true) const;

    unsigned module(unsigned lay, unsigned lad, unsigned mod) const;

    unsigned superstrip_luciano(unsigned lay, float phi, float eta,
                                const float unit_phi, const float unit_eta) const;

    // Debug
    void print();

  private:
    // Initialize
    void init();

  private:
    // Member data
    const unsigned barrel_n_;  // = 6 layers
    const unsigned endcap_n_;  // = 15 rings

    // Variable size
    std::vector<unsigned> barrel_subladder_sizes_;
    std::vector<unsigned> barrel_submodule_sizes_;
    std::vector<unsigned> endcap_subladder_sizes_;
    std::vector<unsigned> endcap_submodule_sizes_;

    // Compress using geometry knowledge
    std::vector<unsigned> barrel_z_divisions_;
    std::vector<unsigned> barrel_phi_divisions_;
    std::vector<unsigned> barrel_layer_offsets_;
    std::vector<unsigned> endcap_ring_divisions_;
    std::vector<unsigned> endcap_ring_offsets_;
    std::vector<unsigned> calo_offsets_;
    std::vector<unsigned> muon_offsets_;
    std::vector<unsigned> fake_offsets_;

    // Number of bits allocated
    unsigned max_subladder_bits_;
    unsigned max_submodule_bits_;
};

}  // namespace slhcl1tt

#endif
