#ifndef AMSimulation_TrackFitterAlgoRetina_h_
#define AMSimulation_TrackFitterAlgoRetina_h_

#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoBase.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Retina.h"
using namespace slhcl1tt;

#include <map>

//
// Implementation of a track fitter based on an artificial retina
//


class TrackFitterAlgoRetina : public TrackFitterAlgoBase
{
 private:

  // This contains the retina configuration parameters,
  // it is filled in initialize():
  std::map <const char*,double> config[3];

  // This is used to rotate the trigger tower into the first quadrant:
  const double rot_angle[8] = {
     0.39269908169872414,   //  pi/8
    -0.39269908169872414,   // -pi/8
    -1.17809724509617242,   // -3/8 pi
    -1.96349540849362070,   // -5/8 pi
    -2.74889357189106898,   // -7/8 pi
    -3.53429173528851726,   // -9/8 pi
    -4.31968989868596509,   // -11/8 pi
    -5.10508806208341426    // -13/8 pi
  };

  void initialize();
  void rotateHits(std::vector<xyPoint>& hits, double angle);
  void confTrans(std::vector<xyPoint>& hits);


 public:

  TrackFitterAlgoRetina();

  virtual ~TrackFitterAlgoRetina();

  //int fit(std::vector<TTHit>& hits, std::vector<TTTrack2>& tracks);
  // virtual int fit(unsigned tower, unsigned eventNum, unsigned roadRef, const std::vector<TTHit>& hits, std::vector<TTTrack2>& tracks);
  virtual int fit(const std::vector<TTHit>& hits, std::vector<TTTrack2>& tracks, unsigned tower, unsigned roadRef);

};

#endif
