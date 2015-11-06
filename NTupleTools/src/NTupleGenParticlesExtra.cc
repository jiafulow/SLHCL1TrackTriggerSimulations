#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/NTupleGenParticlesExtra.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/TrajectoryState/interface/TrackCharge.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/PatternTools/interface/TSCBLBuilderNoMaterial.h"
#include "TrackingTools/PatternTools/interface/TSCPBuilderNoMaterial.h"


namespace {
// Copied from DataFormats/TrackReco/interface/TrackBase.h
double get_dxy(const GlobalPoint& v, const GlobalVector& p) {
    return (-v.x()*p.y() + v.y()*p.x()) / p.perp();
    //return (-vx() * py() + vy() * px()) / pt();
}

double get_d0(const GlobalPoint& v, const GlobalVector& p) {
    return -1.0 * get_dxy(v, p);
}

double get_dsz(const GlobalPoint& v, const GlobalVector& p) {
    return v.z() * p.perp() / p.mag() - (v.x()*p.x() + v.y()*p.y()) / p.perp() * (p.z() / p.mag());
    //return vz() * pt() / p() - (vx() * px() + vy() * py()) / pt() * pz() / p();
}

double get_dz(const GlobalPoint& v, const GlobalVector& p) {
    return v.z() - (v.x()*p.x() + v.y()*p.y()) / p.perp() * (p.z() / p.perp());
    //return vz() - (vx() * px() + vy() * py()) / pt() * (pz() / pt());
}
}

NTupleGenParticlesExtra::NTupleGenParticlesExtra(const edm::ParameterSet& iConfig) :
  inputTag_        (iConfig.getParameter<edm::InputTag>("inputTag")),
  inputTagBeamSpot_(iConfig.getParameter<edm::InputTag>("inputTagBeamSpot")),
  prefix_          (iConfig.getParameter<std::string>("prefix")),
  suffix_          (iConfig.getParameter<std::string>("suffix")),
  selector_        (iConfig.existsAs<std::string>("cut") ? iConfig.getParameter<std::string>("cut") : "", true),
  maxN_            (iConfig.getParameter<unsigned>("maxN")) {

    //produces<std::vector<float> > (prefix_ + "phi"      + suffix_);

    //produces<std::vector<float> > (prefix_ + "qoverp"   + suffix_);
    //produces<std::vector<float> > (prefix_ + "lambda"   + suffix_);
    //produces<std::vector<float> > (prefix_ + "dxy"      + suffix_);
    //produces<std::vector<float> > (prefix_ + "dsz"      + suffix_);

    produces<std::vector<float> > (prefix_ + "invPt"    + suffix_);
    produces<std::vector<float> > (prefix_ + "cotTheta" + suffix_);
    produces<std::vector<float> > (prefix_ + "d0"       + suffix_);
    produces<std::vector<float> > (prefix_ + "dz"       + suffix_);
}

void NTupleGenParticlesExtra::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    /// Magnetic field setup
    edm::ESHandle<MagneticField> magneticFieldHandle;
    iSetup.get<IdealMagneticFieldRecord>().get(magneticFieldHandle);
    theMagneticField = magneticFieldHandle.product();
}

void NTupleGenParticlesExtra::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    //std::auto_ptr<std::vector<float> > v_phi      (new std::vector<float>());

    //std::auto_ptr<std::vector<float> > v_qoverp   (new std::vector<float>());
    //std::auto_ptr<std::vector<float> > v_lambda   (new std::vector<float>());
    //std::auto_ptr<std::vector<float> > v_dxy      (new std::vector<float>());
    //std::auto_ptr<std::vector<float> > v_dsz      (new std::vector<float>());

    std::auto_ptr<std::vector<float> > v_invPt    (new std::vector<float>());
    std::auto_ptr<std::vector<float> > v_cotTheta (new std::vector<float>());
    std::auto_ptr<std::vector<float> > v_d0       (new std::vector<float>());
    std::auto_ptr<std::vector<float> > v_dz       (new std::vector<float>());

    //__________________________________________________________________________
    if (!iEvent.isRealData()) {
        edm::Handle<reco::GenParticleCollection> parts;
        iEvent.getByLabel(inputTag_, parts);

        edm::Handle<reco::BeamSpot> beamSpot;
        iEvent.getByLabel(inputTagBeamSpot_, beamSpot);

        if (parts.isValid()) {
            edm::LogInfo("NTupleGenParticlesExtra") << "Size: " << parts->size();

            unsigned n = 0;
            for (reco::GenParticleCollection::const_iterator it = parts->begin(); it != parts->end(); ++it) {
                if (n >= maxN_)
                    break;
                if (!selector_(*it))
                    continue;

                // theta & lambda
                double theta    = it->theta();
                double lambda   = M_PI_2 - theta;
                double cotTheta = 1.0 / std::tan(theta);

                // Beam spot
                GlobalPoint bsPosition(0., 0., 0.);
                if (beamSpot.isValid()) {
                    bsPosition = GlobalPoint(beamSpot->x0(), beamSpot->y0(), beamSpot->z0());
                }

                // Point of closest approach
                GlobalPoint poca(0., 0., 0.);
                GlobalVector poca_p(0., 0., 0.);

                GlobalPoint vertex(it->vx(), it->vy(), it->vz());
                GlobalVector momentum(it->px(), it->py(), it->pz());
                TrackCharge charge(it->charge());
                FreeTrajectoryState ftsAtProduction(vertex, momentum, charge, theMagneticField);

                // With respect to beam line
                //TrajectoryStateClosestToBeamLine traj = tk.stateAtBeamLine();
                //d0 = traj.transverseImpactParameter().value();
                //z0 = traj.longitudinalImpactParameter().value();

                TSCBLBuilderNoMaterial tscblBuilder;
                TrajectoryStateClosestToBeamLine tsAtClosestApproach = tscblBuilder(ftsAtProduction, *beamSpot);
                if (tsAtClosestApproach.isValid()) {
                    //poca = tsAtClosestApproach.trackStateAtPCA().position() - bsPosition;  // not supported!?
                    poca = tsAtClosestApproach.trackStateAtPCA().position();
                    poca = GlobalPoint(poca.x() - bsPosition.x(), poca.y() - bsPosition.y(), poca.z() - bsPosition.z());
                    poca_p = tsAtClosestApproach.trackStateAtPCA().momentum();
                }

                //// With respect to any specified vertex, such as primary vertex
                ////TrajectoryStateClosestToPoint traj = tk.trajectoryStateClosestToPoint(vertex);
                ////d0 = traj.perigeeParameters().transverseImpactParameter();
                ////z0 = traj.perigeeParameters().longitudinalImpactParameter();

                //TSCPBuilderNoMaterial tscpBuilder;
                //TrajectoryStateClosestToPoint tsAtClosestApproach = tscpBuilder(ftsAtProduction, bsPosition);
                //if (tsAtClosestApproach.isValid()) {
                //    //poca = tsAtClosestApproach.theState().position() - bsPosition;  // not supported!?
                //    poca = tsAtClosestApproach.theState().position();
                //    poca = GlobalPoint(poca.x() - bsPosition.x(), poca.y() - bsPosition.y(), poca.z() - bsPosition.z());
                //    poca_p = tsAtClosestApproach.theState().momentum();
                //}

                // Fill the vectors
                //v_phi      ->push_back(it->phi());

                //v_qoverp   ->push_back(float(it->charge()) / it->p());
                //v_lambda   ->push_back(lambda);
                //v_dxy      ->push_back(get_dxy(poca, poca_p));
                //v_dsz      ->push_back(get_dsz(poca, poca_p));

                v_invPt    ->push_back(float(it->charge()) / it->pt());
                v_cotTheta ->push_back(cotTheta);
                v_d0       ->push_back(get_d0(poca, poca_p));
                v_dz       ->push_back(get_dz(poca, poca_p));

                n++;
            }

        } else {
            edm::LogError("NTupleGenParticlesExtra") << "Cannot get the product: " << inputTag_;
        }
    }

    //__________________________________________________________________________
    //iEvent.put(v_phi      , prefix_ + "phi"      + suffix_);

    //iEvent.put(v_qoverp   , prefix_ + "qoverp"   + suffix_);
    //iEvent.put(v_lambda   , prefix_ + "lambda"   + suffix_);
    //iEvent.put(v_dxy      , prefix_ + "dxy"      + suffix_);
    //iEvent.put(v_dsz      , prefix_ + "dsz"      + suffix_);

    iEvent.put(v_invPt    , prefix_ + "invPt"    + suffix_);
    iEvent.put(v_cotTheta , prefix_ + "cotTheta" + suffix_);
    iEvent.put(v_d0       , prefix_ + "d0"       + suffix_);
    iEvent.put(v_dz       , prefix_ + "dz"       + suffix_);
}
