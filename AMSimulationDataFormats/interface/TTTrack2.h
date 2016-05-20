#ifndef AMSimulationDataFormats_TTTrack2_h_
#define AMSimulationDataFormats_TTTrack2_h_

#include <cmath>
#include <vector>
#include <iosfwd>

namespace slhcl1tt {

// A thinner version of TTTrack
class TTTrack2 {
  public:
    // Constructors
    TTTrack2()
    : rinv_(-999999.), phi0_(-999999.), cottheta_(-999999.), z0_(-999999.), d0_(-999999.),
      chi2_(-999999.), ndof_(-1), chi2_phi_(-999999.), chi2_z_(-999999.), 
      matchChi2_(-1),
      isGhost_(false), tpId_(-1), synTpId_(-2), tower_(99), hitBits_(0), ptSegment_(0), roadRef_(0), combRef_(0), patternRef_(0),
      stubRefs_(), principals_() {}

    TTTrack2(const TTTrack2& rhs)
    : rinv_(rhs.rinv_), phi0_(rhs.phi0_), cottheta_(rhs.cottheta_), z0_(rhs.z0_), d0_(rhs.d0_),
      chi2_(rhs.chi2_), ndof_(rhs.ndof_), chi2_phi_(rhs.chi2_phi_), chi2_z_(rhs.chi2_z_), 
      matchChi2_(rhs.matchChi2_),
      isGhost_(rhs.isGhost_), tpId_(rhs.tpId_), synTpId_(rhs.synTpId_), tower_(rhs.tower_), hitBits_(rhs.hitBits_), ptSegment_(rhs.ptSegment_), roadRef_(rhs.roadRef_), combRef_(rhs.combRef_), patternRef_(rhs.patternRef_),
      stubRefs_(rhs.stubRefs_), principals_(rhs.principals_) {}

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

    void setMatchChi2(float matchChi2)	    		    { matchChi2_ = matchChi2; }
    void setAsGhost()                                       { isGhost_ = true; }
    void setTpId(int tpId)                                  { tpId_ = tpId; }
    void setSynTpId(int synTpId)                            { synTpId_ = synTpId; }
    void setTower(unsigned tower)                           { tower_ = tower; }
    void setHitBits(unsigned hitBits)                       { hitBits_ = hitBits; }
    void setPtSegment(unsigned ptSegment)                   { ptSegment_ = ptSegment; }
    void setRoadRef(unsigned roadRef)                       { roadRef_ = roadRef; }
    void setCombRef(unsigned combRef)                       { combRef_ = combRef; }
    void setPatternRef(unsigned patternRef)                 { patternRef_ = patternRef; }

    void addStubRef(unsigned stubRef)                       { stubRefs_.push_back(stubRef); }
    void setStubRefs(const std::vector<unsigned>& stubRefs) { stubRefs_ = stubRefs; }

    void addPrincipal(float principal)                      { principals_.push_back(principal); }
    void setPrincipals(const std::vector<float>& principals){ principals_ = principals; }

    // Getters
    float rinv()                                const { return rinv_; }

    float phi0()                                const { return phi0_; }

    float cottheta()                            const { return cottheta_; }

    float z0()                                  const { return z0_; }

    float d0()                                  const { return d0_; }

    float chi2()                                const { return chi2_; }

    int   ndof()                                const { return ndof_; }

    float chi2Red()                             const { return chi2() / ndof(); }

    float chi2_phi()                            const { return chi2_phi_; }

    float chi2_z()                              const { return chi2_z_; }

    float matchChi2()				const { return matchChi2_; }

    bool  isGhost()                             const { return isGhost_; }

    int   tpId()                                const { return tpId_; }

    int   synTpId()                             const { return synTpId_; }

    unsigned tower()                            const { return tower_; }

    unsigned hitBits()                          const { return hitBits_; }

    unsigned ptSegment()                        const { return ptSegment_; }

    unsigned roadRef()                          const { return roadRef_; }

    unsigned combRef()                          const { return combRef_; }

    unsigned patternRef()                       const { return patternRef_; }

    std::vector<unsigned> stubRefs()            const { return stubRefs_; }
    unsigned stubRef(int l)                     const { return stubRefs_.at(l); }

    std::vector<float> principals()             const { return principals_; }
    float principal(int l)                      const { return principals_.at(l); }

    float pt(float B=3.8)                       const { return std::abs(0.003 * B / rinv()); }  // assume r is in cm, B is in Tesla
    float invPt(float B=3.8)                    const { return rinv() / (0.003 * B); }          // assume r is in cm, B is in Tesla
    float theta()                               const { return std::atan(1.0 / cottheta()); }
    float eta()                                 const { return -std::log(tan(theta()/2.0)); }
    float phi()                                 const { return phi0(); }
    float px()                                  const { return pt() * std::cos(phi0()); }
    float py()                                  const { return pt() * std::sin(phi0()); }
    float pz()                                  const { return pt() * cottheta(); }
    float vx()                                  const { return 0.; }  // dummy
    float vy()                                  const { return 0.; }  // dummy
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
    float matchChi2_;
    bool  isGhost_;
    int   tpId_;
    int   synTpId_;
    unsigned tower_;
    unsigned hitBits_;
    unsigned ptSegment_;
    unsigned roadRef_;
    unsigned combRef_;
    unsigned patternRef_;
    std::vector<unsigned> stubRefs_;
    std::vector<float>    principals_;
};

// _____________________________________________________________________________
// Output streams
std::ostream& operator<<(std::ostream& o, const TTTrack2& track);

}  // namespace slhcl1tt

#endif
