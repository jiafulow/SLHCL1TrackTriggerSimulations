#ifndef AMSimulationDataFormats_TTTrack_h_
#define AMSimulationDataFormats_TTTrack_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/Vector.h"

#include <vector>

namespace slhcl1tt {

// This is cloned from DataFormats/L1TrackTrigger/interface/TTTrack.h
// with some style fixes.
class TTTrack {
  public:
    /// Constructors
    TTTrack();
    TTTrack(const std::vector<unsigned int>& aStubs);

    /// Destructor
    ~TTTrack() {}

    /// Track components
    std::vector<unsigned int> getStubRefs() const  { return theStubRefs_; }
    void addStubRef(unsigned int aStub)            { theStubRefs_.push_back(aStub); }
    void setStubRefs(const std::vector<unsigned int>& aStubs) { theStubRefs_ = aStubs; }

    /// Track momentum
    GlobalVector getMomentum(unsigned int nPar=5) const;
    void         setMomentum(const GlobalVector& aMomentum, unsigned int nPar=5);

    /// Track curvature
    double       getRInv(unsigned int nPar=5) const;
    void         setRInv(double aRInv, unsigned int nPar=5);

    /// POCA
    GlobalPoint  getPOCA(unsigned int nPar=5) const;
    void         setPOCA(const GlobalPoint& aPOCA, unsigned int nPar=5);

    /// Sector
    unsigned int getSector() const                 { return theSector_; }
    void         setSector(unsigned int aSector)   { theSector_ = aSector; }
    unsigned int getWedge() const                  { return theWedge_; }
    void         setWedge(unsigned int aWedge)     { theWedge_ = aWedge; }

    /// Chi2
    double       getChi2(unsigned int nPar=5) const;
    double       getChi2Red(unsigned int nPar=5) const;
    void         setChi2(double aChi2, unsigned int nPar=5);

    /// Stub Pt consistency
    double       getStubPtConsistency(unsigned int nPar=5) const;
    void         setStubPtConsistency(double aPtConsistency, unsigned int nPar=5);
    
    // Raw track parameters
    double       rho_, phi0_, z0_, cottheta0_, chi2_, chi2Red_;
    
    // Road index. Replaces the list of stubs.
    unsigned int roadIndex_; // We won't need the stubs
    void         setRoadIndex(unsigned int);
    unsigned int getRoadIndex() const;

  private:
    bool checkValidArgs(unsigned int nPar, const std::string& fcn) const;
    bool checkValidArgsForSet(unsigned int nPar, const std::string& fcn) const;

  private:
    std::vector<unsigned> theStubRefs_;
    GlobalVector theMomentum4Par_;
    GlobalVector theMomentum5Par_;
    GlobalPoint  thePOCA4Par_;
    GlobalPoint  thePOCA5Par_;
    double       theRInv4Par_;
    double       theRInv5Par_;
    unsigned int theSector_;
    unsigned int theWedge_;
    double       theChi24Par_;
    double       theChi25Par_;
    double       theStubPtConsistency4Par_;
    double       theStubPtConsistency5Par_;
    bool         valid4ParFit_;
    bool         valid5ParFit_;
};

}  // namespace slhcl1tt

#endif
