#include "SLHCL1TrackTriggerSimulations/AMSimulationDataFormats/interface/TTTrack.h"


namespace slhcl1tt {

/// Default Constructor
TTTrack::TTTrack()
: theStubRefs_(),
  theMomentum4Par_(0.0,0.0,0.0),
  theMomentum5Par_(0.0,0.0,0.0),
  thePOCA4Par_(0.0,0.0,0.0),
  thePOCA5Par_(0.0,0.0,0.0),
  theRInv4Par_(0.0),
  theRInv5Par_(0.0),
  theSector_(0),
  theWedge_(0),
  theChi24Par_(0.0),
  theChi25Par_(0.0),
  theStubPtConsistency4Par_(0.0),
  theStubPtConsistency5Par_(0.0),
  valid4ParFit_(false),
  valid5ParFit_(false) {}

/// Another Constructor
TTTrack::TTTrack(const std::vector<unsigned int>& aStubs)
: theStubRefs_(aStubs),
  theMomentum4Par_(0.0,0.0,0.0),
  theMomentum5Par_(0.0,0.0,0.0),
  thePOCA4Par_(0.0,0.0,0.0),
  thePOCA5Par_(0.0,0.0,0.0),
  theRInv4Par_(0.0),
  theRInv5Par_(0.0),
  theSector_(0),
  theWedge_(0),
  theChi24Par_(0.0),
  theChi25Par_(0.0),
  theStubPtConsistency4Par_(0.0),
  theStubPtConsistency5Par_(0.0),
  valid4ParFit_(false),
  valid5ParFit_(false) {}


void TTTrack::setMomentum(const GlobalVector& aMomentum, unsigned int nPar) {
  if (!checkValidArgsForSet(nPar,"setMomentum")) {
    return;
  }

  if (nPar==4) {
    valid4ParFit_ = true;
    theMomentum4Par_ = aMomentum;
  }

  if (nPar==5) {
    valid5ParFit_ = true;
    theMomentum5Par_ = aMomentum;
  }

  return;
}

GlobalVector TTTrack::getMomentum(unsigned int nPar) const {
  if (!checkValidArgs(nPar,"getMomentum")) {
    return GlobalVector(0.0,0.0,0.0);
  }

  if (nPar==4) {
    return theMomentum4Par_;
  }

  if (nPar==5) {
    return theMomentum5Par_;
  }

  return GlobalVector(0.0,0.0,0.0);
}


void TTTrack::setPOCA(const GlobalPoint& aPOCA, unsigned int nPar) {
  if (!checkValidArgsForSet(nPar,"setPOCA")) {
    return;
  }

  if (nPar==4) {
    valid4ParFit_ = true;
    thePOCA4Par_ = aPOCA;
  }

  if (nPar==5) {
    valid5ParFit_ = true;
    thePOCA5Par_ = aPOCA;
  }

  return;
}

GlobalPoint TTTrack::getPOCA(unsigned int nPar) const {
  if (!checkValidArgs(nPar,"getPOCA")) {
    return GlobalPoint(0.0,0.0,0.0);
  }

  if (nPar==4) {
    return thePOCA4Par_;
  }

  if (nPar==5) {
    return thePOCA5Par_;
  }

  return GlobalPoint(0.0,0.0,0.0);
}


void TTTrack::setRInv(double aRInv, unsigned int nPar) {
  if (!checkValidArgsForSet(nPar,"setRInv")) {
    return;
  }

  if (nPar==4) {
    valid4ParFit_ = true;
    theRInv4Par_ = aRInv;
  }

  if (nPar==5) {
    valid5ParFit_ = true;
    theRInv5Par_ = aRInv;
  }

  return;
}

double TTTrack::getRInv(unsigned int nPar) const {
  if (!checkValidArgs(nPar,"getRInv")) {
    return 0.0;
  }

  if (nPar==4) {
    return theRInv4Par_;
  }

  if (nPar==5) {
    return theRInv5Par_;
  }

  return 0.0;
}


void TTTrack::setChi2(double aChi2, unsigned int nPar) {
  if (!checkValidArgsForSet(nPar,"setChi2")) {
    return;
  }

  if (nPar==4) {
    valid4ParFit_ = true;
    theChi24Par_ = aChi2;
  }

  if (nPar==5) {
    valid5ParFit_ = true;
    theChi25Par_ = aChi2;
  }

  return;
}

double TTTrack::getChi2(unsigned int nPar) const {
  if (!checkValidArgs(nPar,"getChi2")) {
    return 0.0;
  }

  if (nPar==4) {
    return theChi24Par_;
  }

  if (nPar==5) {
    return theChi25Par_;
  }

  return 0.0;
}

double TTTrack::getChi2Red(unsigned int nPar) const {
  if (!checkValidArgs(nPar,"getChi2Red")){
    return 0.0;
  }

  if (nPar==4) {
    return theChi24Par_/(2*theStubRefs_.size() - 4);
  }

  if (nPar==5) {
    return theChi25Par_/(2*theStubRefs_.size() - 5);
  }

  return 0.0;
}


void TTTrack::setStubPtConsistency(double aStubPtConsistency, unsigned int nPar) {
  if (!checkValidArgsForSet(nPar,"setStubPtConsistency")){
    return;
  }

  if (nPar==4) {
    valid4ParFit_ = true;
    theStubPtConsistency4Par_ = aStubPtConsistency;
  }

  if (nPar==5) {
    valid5ParFit_ = true;
    theStubPtConsistency5Par_ = aStubPtConsistency;
  }

  return;
}

double TTTrack::getStubPtConsistency(unsigned int nPar) const {
  if (!checkValidArgs(nPar,"getStubPtConsistency")) {
    return 0.0;
  }

  if (nPar==4) {
    return theStubPtConsistency4Par_;
  }

  if (nPar==5) {
    return theStubPtConsistency5Par_;
  }

  return 0.0;
}

void TTTrack::setRoadIndex(unsigned int i)
{
  roadIndex_=i;
}

unsigned int TTTrack::getRoadIndex() const
{
  return roadIndex_;
}


bool TTTrack::checkValidArgs(unsigned int nPar, const std::string& fcn) const {
  if (!(nPar==4||nPar==5)) {
    return false;
  }

  if ((nPar==4)&&!valid4ParFit_) {
    return false;
  }

  if ((nPar==5)&&!valid5ParFit_) {
    return false;
  }

  return true;
}


bool TTTrack::checkValidArgsForSet(unsigned int nPar, const std::string& fcn) const {
  if (!(nPar==4||nPar==5)) {
    return false;
  }

  return true;
}

}  // namespace slhcl1tt
