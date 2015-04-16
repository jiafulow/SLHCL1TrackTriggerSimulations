#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitterAlgoRetina.h"

#include <iostream>


TrackFitterAlgoRetina::TrackFitterAlgoRetina():TrackFitterAlgoBase(){
  initialize();
}


TrackFitterAlgoRetina::~TrackFitterAlgoRetina(){
}


// int TrackFitterAlgoRetina::fit(unsigned tower, unsigned eventNum, unsigned roadRef, const std::vector<TTHit>& hits, std::vector<TTTrack2>& tracks)
int TrackFitterAlgoRetina::fit(const std::vector<TTHit>& hits, std::vector<TTTrack2>& tracks, unsigned tower, unsigned roadRef){

  int exitcode = 0;

  if(hits.size()>1024){
    std::cout << "ERROR : too many stubs for fitting!" << std::endl;
    return 1;
  }


  //for(unsigned int ihit=0; ihit<hits.size(); ihit++){
  //  std::cout << "  x = " << hits[ihit].x
  //            << "  \ty = " << hits[ihit].y
  //            << "  \tz = " << hits[ihit].z
  //            << "  \trho = " << hits[ihit].r << std::endl;
  //}


  // --- Determine in which phi sector and eta range the trigger tower is:
  const int phi_sector = tower % 8;
  const int eta_range  = tower / 8;

  int trigTow_type = 0;
  if ( eta_range==1 || eta_range==4 )
    trigTow_type = 1;
  else if ( eta_range==0 || eta_range==5 )
    trigTow_type = 2;

  // --- Constants used in X+-X- transformation:
  double y0 =  0.0217391304347826081; // 0.5/23.
  double y1 =  0.0046296296296296294; // 0.5/108.


  // Use xyPoint instead of TTHit
  std::vector < xyPoint > hits_XY;
  std::vector < xyPoint > hits_RZ;

  for (unsigned int ihit=0; ihit<hits.size(); ++ihit){
    hits_XY.emplace_back(xyPoint{hits[ihit].x(), hits[ihit].y(), ihit});
  }


  // ===========================================================================
  //  XY fit
  // ===========================================================================

  // --- Phi sector rotation:
  rotateHits(hits_XY, rot_angle[phi_sector]);


  // --- Conformal transformation:
  confTrans(hits_XY);


  //
  // --- First step ------------------------------------------------------------
  //

  // --- Setup the retina:
  double pbins_step1 = config[trigTow_type]["xy_pbins_step1"];
  double qbins_step1 = config[trigTow_type]["xy_qbins_step1"];
  double pmin_step1  = config[trigTow_type]["xy_pmin_step1"];
  double pmax_step1  = config[trigTow_type]["xy_pmax_step1"];
  double qmin_step1  = config[trigTow_type]["xy_qmin_step1"];
  double qmax_step1  = config[trigTow_type]["xy_qmax_step1"];

  double minWeight_step1 = config[trigTow_type]["xy_threshold_step1"];

  double pstep_step1 = (pmax_step1-pmin_step1)/pbins_step1;
  double qstep_step1 = (qmax_step1-qmin_step1)/qbins_step1;

  std::vector <double> sigma_step1(8,0.25*sqrt(pstep_step1*pstep_step1+qstep_step1*qstep_step1));
  if ( config[trigTow_type]["xy_sigma1_step1"] != 0. )
    sigma_step1[0] = config[trigTow_type]["xy_sigma1_step1"];
  if ( config[trigTow_type]["xy_sigma2_step1"] != 0. )
    sigma_step1[1] = config[trigTow_type]["xy_sigma2_step1"];
  if ( config[trigTow_type]["xy_sigma3_step1"] != 0. )
    sigma_step1[2] = config[trigTow_type]["xy_sigma3_step1"];
  if ( config[trigTow_type]["xy_sigma4_step1"] != 0. )
    sigma_step1[3] = config[trigTow_type]["xy_sigma4_step1"];
  if ( config[trigTow_type]["xy_sigma5_step1"] != 0. )
    sigma_step1[4] = config[trigTow_type]["xy_sigma5_step1"];
  if ( config[trigTow_type]["xy_sigma6_step1"] != 0. )
    sigma_step1[5] = config[trigTow_type]["xy_sigma6_step1"];
  if ( config[trigTow_type]["xy_sigma7_step1"] != 0. )
    sigma_step1[6] = config[trigTow_type]["xy_sigma7_step1"];
  if ( config[trigTow_type]["xy_sigma8_step1"] != 0. )
    sigma_step1[7] = config[trigTow_type]["xy_sigma8_step1"];


  Retina retinaXY_step1(hits_XY, pbins_step1+2, qbins_step1+2,
                        pmin_step1-pstep_step1, pmax_step1+pstep_step1,
                        qmin_step1-qstep_step1, qmax_step1+qstep_step1,
                        sigma_step1, minWeight_step1, XY);

  // --- Fill the retina and find maxima:
  retinaXY_step1.fillGrid();
  retinaXY_step1.findMaxima();
  //retinaXY_step1.dumpGrid(eventNum,1);
  //retinaXY_step1.printMaxima();


  // --- Get first step maxima:
  std::vector <pqPoint> maximaXY_step1 = retinaXY_step1.getMaxima();


  //
  // --- Second step -----------------------------------------------------------
  //

  double pbins_step2 = config[trigTow_type]["xy_pbins_step2"];
  double qbins_step2 = config[trigTow_type]["xy_qbins_step2"];


  // --- Zoom around first step maxima:
  for (unsigned int imax=0; imax<maximaXY_step1.size(); ++imax){
    if (maximaXY_step1[imax].w == -1.) continue;

    // --- Retina setup:
    double pmin_step2 = maximaXY_step1[imax].p - config[trigTow_type]["xy_zoom_step2"]*pstep_step1;
    double pmax_step2 = maximaXY_step1[imax].p + config[trigTow_type]["xy_zoom_step2"]*pstep_step1;
    double qmin_step2 = maximaXY_step1[imax].q - config[trigTow_type]["xy_zoom_step2"]*qstep_step1;
    double qmax_step2 = maximaXY_step1[imax].q + config[trigTow_type]["xy_zoom_step2"]*qstep_step1;

    double pstep_step2 = (pmax_step2-pmin_step2)/pbins_step2;
    double qstep_step2 = (qmax_step2-qmin_step2)/qbins_step2;

    double minWeight_step2 = config[trigTow_type]["xy_threshold_step2"];

    std::vector <double> sigma_step2(8,0.25*sqrt(pstep_step2*pstep_step2+qstep_step2*qstep_step2));
    if ( config[trigTow_type]["xy_sigma1_step2"] != 0. )
      sigma_step2[0] = config[trigTow_type]["xy_sigma1_step2"];
    if ( config[trigTow_type]["xy_sigma2_step2"] != 0. )
      sigma_step2[1] = config[trigTow_type]["xy_sigma2_step2"];
    if ( config[trigTow_type]["xy_sigma3_step2"] != 0. )
      sigma_step2[2] = config[trigTow_type]["xy_sigma3_step2"];
    if ( config[trigTow_type]["xy_sigma4_step2"] != 0. )
      sigma_step2[3] = config[trigTow_type]["xy_sigma4_step2"];
    if ( config[trigTow_type]["xy_sigma5_step2"] != 0. )
      sigma_step2[4] = config[trigTow_type]["xy_sigma5_step2"];
    if ( config[trigTow_type]["xy_sigma6_step2"] != 0. )
      sigma_step2[5] = config[trigTow_type]["xy_sigma6_step2"];
    if ( config[trigTow_type]["xy_sigma7_step2"] != 0. )
      sigma_step2[6] = config[trigTow_type]["xy_sigma7_step2"];
    if ( config[trigTow_type]["xy_sigma8_step2"] != 0. )
      sigma_step2[7] = config[trigTow_type]["xy_sigma8_step2"];


    Retina retinaXY_step2(hits_XY, pbins_step2+2, qbins_step2+2,
                          pmin_step2-pstep_step2, pmax_step2+pstep_step2,
                          qmin_step2-qstep_step2, qmax_step2+qstep_step2,
                          sigma_step2, minWeight_step2, XY);

    // --- Fill the retina and find maxima:
    retinaXY_step2.fillGrid();
    retinaXY_step2.findMaxima();
    //retinaXY_step2.dumpGrid(eventNum,2,imax);
    //retinaXY_step2.printMaxima();

    pqPoint bestpqXY_step2 = retinaXY_step2.getBestPQ();
    if ( bestpqXY_step2.w == -1. ) continue;


    // --- Invert the X+-X- transformation:
    double p = 0.5*(y1 - y0)/bestpqXY_step2.q;
    double q = y0 - p*(bestpqXY_step2.p-bestpqXY_step2.q);


    // --- Associate stubs to this maxumum:
    hits_RZ.clear();
    for (unsigned int ihit=0; ihit<hits.size(); ++ihit){

      double dist   = fabs(hits_XY[ihit].y-p*hits_XY[ihit].x-q)/sqrt(1.+p*p);
      double weight = exp(-0.5*dist*dist/(sigma_step2[0]*sigma_step2[0]));

      if ( weight>0.5 ){
        hits_RZ.emplace_back(xyPoint{fabs(hits[ihit].z), hits[ihit].r, ihit});  // NB: we are using fabs(z) !!!!
      }
      //cout << ihit << " - " << dist << "  " << weight << endl;

    }


    // --- Rotate back the original phi sector:
    q = q/(cos(rot_angle[phi_sector])+p*sin(rot_angle[phi_sector]));
    p = (p*cos(rot_angle[phi_sector])-sin(rot_angle[phi_sector]))/
        (cos(rot_angle[phi_sector])+p*sin(rot_angle[phi_sector]));


    // --- Invert the conformal transformation and get the track parameters:
    double a = -0.5*p/q;
    double b =  0.5/q;

    double c   = 1./sqrt(a*a+b*b);
    double phi = atan(p);
    //if (phi<0.)
    //  phi += TMath::Pi();


    // =========================================================================
    //  RZ fit
    // =========================================================================

    y0 = 0.5/y0;
    y1 = 0.5/y1;

    double cottheta = -9999.;
    double z0       = -9999.;


    //
    // --- First step ----------------------------------------------------------
    //

    pbins_step1 = config[trigTow_type]["rz_pbins_step1"];
    qbins_step1 = config[trigTow_type]["rz_qbins_step1"];
    pmin_step1  = config[trigTow_type]["rz_pmin_step1"];
    pmax_step1  = config[trigTow_type]["rz_pmax_step1"];
    qmin_step1  = config[trigTow_type]["rz_qmin_step1"];
    qmax_step1  = config[trigTow_type]["rz_qmax_step1"];

    minWeight_step1 = config[trigTow_type]["rz_threshold_step1"];

    pstep_step1 = (pmax_step1-pmin_step1)/pbins_step1;
    qstep_step1 = (qmax_step1-qmin_step1)/qbins_step1;

    for (unsigned int ilayer=0; ilayer<8; ++ilayer)
      sigma_step1[ilayer] = 0.5*sqrt(pstep_step1*pstep_step1+qstep_step1*qstep_step1);

    if ( config[trigTow_type]["rz_sigma1_step1"] != 0. )
      sigma_step1[0] = config[trigTow_type]["rz_sigma1_step1"];
    if ( config[trigTow_type]["rz_sigma2_step1"] != 0. )
      sigma_step1[1] = config[trigTow_type]["rz_sigma2_step1"];
    if ( config[trigTow_type]["rz_sigma3_step1"] != 0. )
      sigma_step1[2] = config[trigTow_type]["rz_sigma3_step1"];
    if ( config[trigTow_type]["rz_sigma4_step1"] != 0. )
      sigma_step1[3] = config[trigTow_type]["rz_sigma4_step1"];
    if ( config[trigTow_type]["rz_sigma5_step1"] != 0. )
      sigma_step1[4] = config[trigTow_type]["rz_sigma5_step1"];
    if ( config[trigTow_type]["rz_sigma6_step1"] != 0. )
      sigma_step1[5] = config[trigTow_type]["rz_sigma6_step1"];
    if ( config[trigTow_type]["rz_sigma7_step1"] != 0. )
      sigma_step1[6] = config[trigTow_type]["rz_sigma7_step1"];
    if ( config[trigTow_type]["rz_sigma8_step1"] != 0. )
      sigma_step1[7] = config[trigTow_type]["rz_sigma8_step1"];


    Retina retinaRZ_step1(hits_RZ, pbins_step1+2, qbins_step1+2,
                          pmin_step1-pstep_step1, pmax_step1+pstep_step1,
                          qmin_step1-qstep_step1, qmax_step1+qstep_step1,
                          sigma_step1, minWeight_step1, RZ);

    retinaRZ_step1.fillGrid();
    retinaRZ_step1.findMaxima();
    //retinaRZ_step1.dumpGrid(eventNum,imax);
    //retinaRZ_step1.printMaxima();


    // --- Get first step maximum:
    std::vector <pqPoint> maximaRZ_step1 = retinaRZ_step1.getMaxima();
    //pqPoint bestpqRZ_step1 = retinaRZ_step1.getBestPQ();


    //
    // --- Second step ---------------------------------------------------------
    //

    pbins_step2 = config[trigTow_type]["rz_pbins_step2"];
    qbins_step2 = config[trigTow_type]["rz_qbins_step2"];

    // Zoom around first step maxima
    for (unsigned int imax_RZ=0; imax_RZ<maximaRZ_step1.size(); ++imax_RZ){
      if (maximaRZ_step1[imax].w == -1.) continue;

      double pmin_step2 = maximaRZ_step1[imax_RZ].p - config[trigTow_type]["rz_zoom_step2"]*pstep_step1;
      double pmax_step2 = maximaRZ_step1[imax_RZ].p + config[trigTow_type]["rz_zoom_step2"]*pstep_step1;
      double qmin_step2 = maximaRZ_step1[imax_RZ].q - config[trigTow_type]["rz_zoom_step2"]*qstep_step1;
      double qmax_step2 = maximaRZ_step1[imax_RZ].q + config[trigTow_type]["rz_zoom_step2"]*qstep_step1;

      double pstep_step2 = (pmax_step2-pmin_step2)/pbins_step2;
      double qstep_step2 = (qmax_step2-qmin_step2)/qbins_step2;

      double minWeight_step2 = config[trigTow_type]["rz_threshold_step2"];


      std::vector <double> sigma_step2(8,0.5*sqrt(pstep_step2*pstep_step2+qstep_step2*qstep_step2));
      for (unsigned int ilayer=3; ilayer<6; ++ilayer)
        sigma_step2[ilayer] = 6.*sqrt(pstep_step2*pstep_step2+qstep_step2*qstep_step2);

      if ( config[trigTow_type]["rz_sigma1_step2"] != 0. )
        sigma_step2[0] = config[trigTow_type]["rz_sigma1_step2"];
      if ( config[trigTow_type]["rz_sigma2_step2"] != 0. )
        sigma_step2[1] = config[trigTow_type]["rz_sigma2_step2"];
      if ( config[trigTow_type]["rz_sigma3_step2"] != 0. )
        sigma_step2[2] = config[trigTow_type]["rz_sigma3_step2"];
      if ( config[trigTow_type]["rz_sigma4_step2"] != 0. )
        sigma_step2[3] = config[trigTow_type]["rz_sigma4_step2"];
      if ( config[trigTow_type]["rz_sigma5_step2"] != 0. )
        sigma_step2[4] = config[trigTow_type]["rz_sigma5_step2"];
      if ( config[trigTow_type]["rz_sigma6_step2"] != 0. )
        sigma_step2[5] = config[trigTow_type]["rz_sigma6_step2"];
      if ( config[trigTow_type]["rz_sigma7_step2"] != 0. )
        sigma_step2[6] = config[trigTow_type]["rz_sigma7_step2"];
      if ( config[trigTow_type]["rz_sigma8_step2"] != 0. )
        sigma_step2[7] = config[trigTow_type]["rz_sigma8_step2"];


      Retina retinaRZ_step2(hits_RZ, pbins_step2+2, qbins_step2+2,
                            pmin_step2-pstep_step2, pmax_step2+pstep_step2,
                            qmin_step2-qstep_step2, qmax_step2+qstep_step2,
                            sigma_step2, minWeight_step2, RZ);

      retinaRZ_step2.fillGrid();
      retinaRZ_step2.findMaxima();
      //retinaRZ_step2.dumpGrid(eventNum,2,10+imax_RZ);
      //retinaRZ_step2.printMaxima();

      pqPoint bestpqRZ_step2 = retinaRZ_step2.getBestPQ();
      if (bestpqRZ_step2.w == -1.) continue;

      // --- Invert the X+-X- transformation:
      double p = 0.5*(y1 - y0)/bestpqRZ_step2.q;
      double q = y0 - p*(bestpqRZ_step2.p-bestpqRZ_step2.q);


      // --- Get the track parameters:
      //double theta = atan(p);
      //eta = -log(tan(0.5*theta));
      cottheta = 1./p;
      z0  = -q/p;

      // This is because we fit fabs(z):
      if ( eta_range < 3 ){
        cottheta = -cottheta;
        z0 = -z0;
      }

      if ( !std::isnan(c) && !std::isnan(phi) && !std::isnan(cottheta) && !std::isnan(z0) &&
           cottheta != -9999. && z0 != -9999. ){

        TTTrack2 atrack;
        atrack.setRoadRef(roadRef);
        atrack.setTower(tower);
        atrack.setTrackParams(c, phi, cottheta, z0, 0., 0., 0, 0., 0.);
        for(unsigned int ihit=0; ihit<hits_RZ.size(); ihit++)
          atrack.addStubRef(hits[hits_RZ[ihit].hitRef].ref);

        tracks.push_back(atrack);
      }


    } // imax_RZ loop


  } // imax loop


  return exitcode;
}

void TrackFitterAlgoRetina::rotateHits(std::vector<xyPoint>& hits, double angle){

  for (unsigned int ihit=0; ihit<hits.size(); ihit++) {
    double x = hits[ihit].x*cos(angle) - hits[ihit].y*sin(angle);
    double y = hits[ihit].x*sin(angle) + hits[ihit].y*cos(angle);
    hits[ihit].x = x;
    hits[ihit].y = y;
  }

}

void TrackFitterAlgoRetina::confTrans(std::vector<xyPoint>& hits){

  for (unsigned int ihit=0; ihit<hits.size(); ihit++) {
    double R2 = hits[ihit].x*hits[ihit].x + hits[ihit].y*hits[ihit].y;
    hits[ihit].x /= R2;
    hits[ihit].y /= R2;
  }

}

void TrackFitterAlgoRetina::initialize(){

  // Enter all the retina parameters
  // (we refer to the detector geometry in
  //  http://sviret.web.cern.ch/sviret/Images/CMS/Upgrade/Eta6_Phi8.jpg)

  // --- Central trigger tower:
  config[0]["xy_pbins_step1"]     = 40.;
  config[0]["xy_qbins_step1"]     = 40.;
  config[0]["xy_pmin_step1"]      = -0.05;
  config[0]["xy_pmax_step1"]      =  0.05;
  config[0]["xy_qmin_step1"]      = -0.05;
  config[0]["xy_qmax_step1"]      =  0.05;
  config[0]["xy_threshold_step1"] =  4.;
  config[0]["xy_sigma1_step1"]    =  0.;
  config[0]["xy_sigma2_step1"]    =  0.;
  config[0]["xy_sigma3_step1"]    =  0.;
  config[0]["xy_sigma4_step1"]    =  0.;
  config[0]["xy_sigma5_step1"]    =  0.;
  config[0]["xy_sigma6_step1"]    =  0.;
  config[0]["xy_sigma7_step1"]    =  0.;
  config[0]["xy_sigma8_step1"]    =  0.;
  config[0]["xy_pbins_step2"]     = 100.;
  config[0]["xy_qbins_step2"]     = 100.;
  config[0]["xy_zoom_step2"]      = 1.;
  config[0]["xy_threshold_step2"] =  4.5;
  config[0]["xy_sigma1_step2"]    =  0.;
  config[0]["xy_sigma2_step2"]    =  0.;

  config[0]["rz_pbins_step1"]     = 20.;
  config[0]["rz_qbins_step1"]     = 20.;
  config[0]["rz_pmin_step1"]      = -20.;
  config[0]["rz_pmax_step1"]      =  60.;
  config[0]["rz_qmin_step1"]      = -60.;
  config[0]["rz_qmax_step1"]      =  60.;
  config[0]["rz_threshold_step1"] =  4.;
  config[0]["rz_sigma1_step1"]    =  0.;
  config[0]["rz_sigma2_step1"]    =  0.;
  config[0]["rz_sigma3_step1"]    =  0.;
  config[0]["rz_sigma4_step1"]    =  0.;
  config[0]["rz_sigma5_step1"]    =  0.;
  config[0]["rz_sigma6_step1"]    =  0.;
  config[0]["rz_sigma7_step1"]    =  0.;
  config[0]["rz_sigma8_step1"]    =  0.;
  config[0]["rz_pbins_step2"]     = 80.;
  config[0]["rz_qbins_step2"]     = 80.;
  config[0]["rz_zoom_step2"]      = 1.5;
  config[0]["rz_threshold_step2"] =  4.;
  config[0]["rz_sigma1_step2"]    =  0.;
  config[0]["rz_sigma2_step2"]    =  0.;

  // --- Hybrid trigger tower:
  config[1]["xy_pbins_step1"]     = 40.;
  config[1]["xy_qbins_step1"]     = 40.;
  config[1]["xy_pmin_step1"]      = -0.05;
  config[1]["xy_pmax_step1"]      =  0.05;
  config[1]["xy_qmin_step1"]      = -0.05;
  config[1]["xy_qmax_step1"]      =  0.05;
  config[1]["xy_threshold_step1"] =  4.5;
  config[1]["xy_sigma1_step1"]    =  0.;
  config[1]["xy_sigma2_step1"]    =  0.;
  config[1]["xy_sigma3_step1"]    =  0.;
  config[1]["xy_sigma4_step1"]    =  0.;
  config[1]["xy_sigma5_step1"]    =  0.;
  config[1]["xy_sigma6_step1"]    =  0.;
  config[1]["xy_sigma7_step1"]    =  0.;
  config[1]["xy_sigma8_step1"]    =  0.;
  config[1]["xy_pbins_step2"]     = 100.;
  config[1]["xy_qbins_step2"]     = 100.;
  config[1]["xy_zoom_step2"]      = 1.;
  config[1]["xy_threshold_step2"] =  4.5;
  config[1]["xy_sigma1_step2"]    =  0.;
  config[1]["xy_sigma2_step2"]    =  0.;

  config[1]["rz_pbins_step1"]     = 20.;
  config[1]["rz_qbins_step1"]     = 20.;
  config[1]["rz_pmin_step1"]      = 40.;
  config[1]["rz_pmax_step1"]      = 140.;
  config[1]["rz_qmin_step1"]      = 0.;
  config[1]["rz_qmax_step1"]      = 120.;
  config[1]["rz_threshold_step1"] =  4.5;
  config[1]["rz_sigma1_step1"]    =  0.;
  config[1]["rz_sigma2_step1"]    =  0.;
  config[1]["rz_sigma3_step1"]    =  0.;
  config[1]["rz_sigma4_step1"]    =  0.;
  config[1]["rz_sigma5_step1"]    =  0.;
  config[1]["rz_sigma6_step1"]    =  0.;
  config[1]["rz_sigma7_step1"]    =  0.;
  config[1]["rz_sigma8_step1"]    =  0.;
  config[1]["rz_pbins_step2"]     = 80.;
  config[1]["rz_qbins_step2"]     = 80.;
  config[1]["rz_zoom_step2"]      = 1.5;
  config[1]["rz_threshold_step2"] =  4.5;
  config[1]["rz_sigma1_step2"]    =  0.;
  config[1]["rz_sigma2_step2"]    =  0.;

  // --- Forward trigger tower:
  config[2]["xy_pbins_step1"]     = 40.;
  config[2]["xy_qbins_step1"]     = 40.;
  config[2]["xy_pmin_step1"]      = -10.05;
  config[2]["xy_pmax_step1"]      =  10.05;
  config[2]["xy_qmin_step1"]      = -10.05;
  config[2]["xy_qmax_step1"]      =  10.05;
  config[2]["xy_threshold_step1"] =  4.5;
  config[2]["xy_sigma1_step1"]    =  0.;
  config[2]["xy_sigma2_step1"]    =  0.;
  config[2]["xy_sigma3_step1"]    =  0.;
  config[2]["xy_sigma4_step1"]    =  0.;
  config[2]["xy_sigma5_step1"]    =  0.;
  config[2]["xy_sigma6_step1"]    =  0.;
  config[2]["xy_sigma7_step1"]    =  0.;
  config[2]["xy_sigma8_step1"]    =  0.;
  config[2]["xy_pbins_step2"]     = 100.;
  config[2]["xy_qbins_step2"]     = 100.;
  config[2]["xy_zoom_step2"]      = 1.;
  config[2]["xy_threshold_step2"] =  4.5;
  config[2]["xy_sigma1_step2"]    =  0.;
  config[2]["xy_sigma2_step2"]    =  0.;

  config[2]["rz_pbins_step1"]     = 20.;
  config[2]["rz_qbins_step1"]     = 20.;
  config[2]["rz_pmin_step1"]      = 100.;
  config[2]["rz_pmax_step1"]      = 300.;
  config[2]["rz_qmin_step1"]      = 100.;
  config[2]["rz_qmax_step1"]      = 300.;
  config[2]["rz_threshold_step1"] =  4.5;
  config[2]["rz_sigma1_step1"]    =  0.;
  config[2]["rz_sigma2_step1"]    =  0.;
  config[2]["rz_sigma3_step1"]    =  0.;
  config[2]["rz_sigma4_step1"]    =  0.;
  config[2]["rz_sigma5_step1"]    =  0.;
  config[2]["rz_sigma6_step1"]    =  0.;
  config[2]["rz_sigma7_step1"]    =  0.;
  config[2]["rz_sigma8_step1"]    =  0.;
  config[2]["rz_pbins_step2"]     = 80.;
  config[2]["rz_qbins_step2"]     = 80.;
  config[2]["rz_zoom_step2"]      = 1.5;
  config[2]["rz_threshold_step2"] = 4.5;
  config[2]["rz_sigma1_step2"]    = 0.;
  config[2]["rz_sigma2_step2"]    = 0.;


}
