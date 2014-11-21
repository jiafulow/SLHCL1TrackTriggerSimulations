#ifndef AMSimulationDataFormats_TTTrack2_h_
#define AMSimulationDataFormats_TTTrack2_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Helper.h"
#include <iosfwd>
#include <vector>

namespace slhcl1tt {

// A thinner version of TTTrack
class TTTrack2 {
  public:
    // Constructors
    TTTrack2()
    : rinv_(-999999.), phi0_(-999999.), cottheta_(-999999.), z0_(-999999.), d0_(-999999.),
      chi2_(-999999.), ndof_(-1), chi2_phi_(-999999.), chi2_z_(-999999.),
      tpId_(-1),
      roadRef_(), combRef_(), stubRefs_() {}

    TTTrack2(unsigned roadRef, unsigned combRef, const std::vector<unsigned>& stubRefs)
    : rinv_(-999999.), phi0_(-999999.), cottheta_(-999999.), z0_(-999999.), d0_(-999999.),
      chi2_(-999999.), ndof_(-1), chi2_phi_(-999999.), chi2_z_(-999999.),
      tpId_(-1),
      roadRef_(roadRef), combRef_(combRef), stubRefs_(stubRefs) {}

    TTTrack2(const TTTrack2& rhs)
    : rinv_(rhs.rinv_), phi0_(rhs.phi0_), cottheta_(rhs.cottheta_), z0_(rhs.z0_), d0_(rhs.d0_),
      chi2_(rhs.chi2_), ndof_(rhs.ndof_), chi2_phi_(rhs.chi2_phi_), chi2_z_(rhs.chi2_z_),
      tpId_(rhs.tpId_),
      roadRef_(rhs.roadRef_), combRef_(rhs.combRef_), stubRefs_(rhs.stubRefs_) {}

    // Destructor
    ~TTTrack2() {}

    // Setters
    void setTrackParams(float rinv, float phi0, float cottheta, float z0, float d0,
                        float chi2, int ndof, float chi2_phi, float chi2_z) {
        rinv_     = rinv;
        phi0_     = phi0;
        cottheta_ = cottheta;
        z0_       = z0;
        d0_       = d0;
        chi2_     = chi2;
        ndof_     = ndof;
        chi2_phi_ = chi2_phi;
        chi2_z_   = chi2_z;
    }

    void setTpId(int tpId) {
        tpId_     = tpId;
    }

    // Getters
    float rinv()                                const { return rinv_; }

    float phi0()                                const { return phi0_; }

    float cottheta()                            const { return cottheta_; }

    float z0()                                  const { return z0_; }

    float d0()                                  const { return d0_; }

    float chi2()                                const { return chi2_; }

    int   ndof()                                const { return ndof_; }

    float chi2_phi()                            const { return chi2_phi_; }

    float chi2_z()                              const { return chi2_z_; }

    int   tpId()                                const { return tpId_; }

    unsigned roadRef()                          const { return roadRef_; }

    unsigned combRef()                          const { return combRef_; }

    std::vector<unsigned> stubRefs()            const { return stubRefs_; }
    unsigned stubRef(int l)                     const { return stubRefs_.at(l); }

    float pt(float B=3.8)                       const { return std::abs(0.003 * B / rinv()); }
    float theta()                               const { return std::atan(1.0 / cottheta()); }
    float eta()                                 const { return -std::log(tan(theta()/2.0)); }
    float phi()                                 const { return phi0(); }
    float px()                                  const { return pt() * cos(phi0()); }
    float py()                                  const { return pt() * sin(phi0()); }
    float pz()                                  const { return pt() * cottheta(); }
    float vx()                                  const { return 0.; }
    float vy()                                  const { return 0.; }
    float vz()                                  const { return z0(); }


  private:
    float rinv_;
    float phi0_;
    float cottheta_;
    float z0_;
    float d0_;
    float chi2_;
    int   ndof_;
    float chi2_phi_;
    float chi2_z_;
    int   tpId_;
    unsigned roadRef_;
    unsigned combRef_;
    std::vector<unsigned> stubRefs_;
};

// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const TTTrack2& track);

}  // namespace slhcl1tt

#endif
