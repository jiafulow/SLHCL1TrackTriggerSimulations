#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Framework/interface/EDAnalyzer.h"
//#include "FWCore/ServiceRegistry/interface/Service.h"
//#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "Geometry/Records/interface/StackedTrackerGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/StackedTrackerGeometry.h"
#include "Geometry/TrackerGeometryBuilder/interface/StackedTrackerDetUnit.h"
#include "DataFormats/SiPixelDetId/interface/StackedTrackerDetId.h"
//#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/ModuleIdFunctor.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "TString.h"


class AnalyzerModuleCoordinates : public edm::EDAnalyzer {
  public:
    /// Constructor/destructor
    explicit AnalyzerModuleCoordinates(const edm::ParameterSet&);
    virtual ~AnalyzerModuleCoordinates();

  private:
    virtual void beginRun(const edm::Run&, const edm::EventSetup&);
    virtual void endRun(const edm::Run&, const edm::EventSetup&);

    virtual void beginJob();
    virtual void endJob();
    virtual void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  private:
    /// For event setup
    const TrackerGeometry * theGeometry;
    const StackedTrackerGeometry * theStackedGeometry;

    /// Maps
    std::map<uint32_t, uint32_t> moduleId0ToStackId;
    std::map<uint32_t, uint32_t> moduleId1ToStackId;
    std::map<uint32_t, uint32_t> moduleId0ToGeoId;
    std::map<uint32_t, uint32_t> moduleId1ToGeoId;

    /// Configurations
    std::string csvfile_;
    int verbose_;
};


AnalyzerModuleCoordinates::AnalyzerModuleCoordinates(const edm::ParameterSet& iConfig)
: csvfile_(iConfig.getParameter<std::string>("csv") ),
  verbose_(iConfig.getParameter<int>("verbosity") ) {}

AnalyzerModuleCoordinates::~AnalyzerModuleCoordinates() {}


// Here we make the layout of the detector
void AnalyzerModuleCoordinates::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    /// Geometry setup
    edm::ESHandle<TrackerGeometry> geometryHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geometryHandle);
    theGeometry = geometryHandle.product();

    edm::ESHandle<StackedTrackerGeometry> stackedGeometryHandle;
    iSetup.get<StackedTrackerGeometryRecord>().get(stackedGeometryHandle);
    theStackedGeometry = stackedGeometryHandle.product();

    /// Clear maps
    moduleId0ToStackId.clear();
    moduleId1ToStackId.clear();
    moduleId0ToGeoId.clear();
    moduleId1ToGeoId.clear();

    /// Prepare detId -> moduleId
    ModuleIdFunctor getModuleId;

    /// Loop over the detector elements
    StackedTrackerGeometry::StackContainerIterator  stkIterator;
    for (stkIterator = theStackedGeometry->stacks().begin();
         stkIterator != theStackedGeometry->stacks().end();
         ++stkIterator) {

        StackedTrackerDetUnit* stackDetUnit = *stkIterator;
        StackedTrackerDetId stackDetId = stackDetUnit->Id();
        assert(stackDetUnit == theStackedGeometry->idToStack(stackDetId));

        /// GeomDet and GeomDetUnit are needed to access each
        /// DetId and topology and geometric features
        /// Convert to specific DetId
        const GeomDet* det0 = theStackedGeometry->idToDet(stackDetId, 0);
        const GeomDet* det1 = theStackedGeometry->idToDet(stackDetId, 1);
        //const GeomDetUnit* detUnit0 = theStackedGeometry->idToDetUnit(stackDetId, 0);
        //const GeomDetUnit* detUnit1 = theStackedGeometry->idToDetUnit(stackDetId, 1);

        const DetId geoId0 = det0->geographicalId();
        const DetId geoId1 = det1->geographicalId();

        uint32_t moduleId0 = getModuleId(geoId0);
        uint32_t moduleId1 = getModuleId(geoId1);
        assert(moduleId0 == moduleId1);

        if (moduleId0ToGeoId.find(moduleId0) == moduleId0ToGeoId.end() )
            moduleId0ToGeoId.insert(std::make_pair(moduleId0, geoId0.rawId()) );
        else
            std::cout << "Error: This pair already exists in map! moduleId0: " << moduleId0 << " geoId0: " << geoId0.rawId() << " existing value in map: " << moduleId0ToGeoId.at(moduleId0) << std::endl;

        if (moduleId1ToGeoId.find(moduleId1) == moduleId1ToGeoId.end() )
            moduleId1ToGeoId.insert(std::make_pair(moduleId1, geoId1.rawId()) );
        else
            std::cout << "Error: This pair already exists in map! moduleId1: " << moduleId1 << " geoId1: " << geoId1.rawId() << " existing value in map: " << moduleId1ToGeoId.at(moduleId1) << std::endl;

        if (moduleId0ToStackId.find(moduleId0) == moduleId0ToStackId.end() )
            moduleId0ToStackId.insert(std::make_pair(moduleId0, stackDetId.rawId()) );
        else
            std::cout << "Error: This pair already exists in map! moduleId0: " << moduleId0 << " stackId: " << stackDetId.rawId() << " existing value in map: " << moduleId0ToStackId.at(moduleId0) << std::endl;

        if (moduleId1ToStackId.find(moduleId1) == moduleId1ToStackId.end() )
            moduleId1ToStackId.insert(std::make_pair(moduleId1, stackDetId.rawId()) );
        else
            std::cout << "Error: This pair already exists in map! moduleId1: " << moduleId1 << " stackId: " << stackDetId.rawId() << " existing value in map: " << moduleId1ToStackId.at(moduleId1) << std::endl;

    }  // end loop over stacks

}

void AnalyzerModuleCoordinates::endRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {}


void AnalyzerModuleCoordinates::beginJob() {}


// Here we write into the text file
void AnalyzerModuleCoordinates::endJob() {
    std::cout << "Map size: " << moduleId0ToStackId.size() << ", " << moduleId1ToStackId.size()
              << ", " << moduleId0ToGeoId.size() << ", " << moduleId1ToGeoId.size() << std::endl;

    // Open text file
    ofstream csvfile(csvfile_);

    // Write text file
    //int i=0;
    csvfile << "moduleId/I, z_cm/D, r_cm/D, eta/D, phi/D, sensorSpacing_cm/D" << std::endl;

    for (const auto& kv : moduleId0ToStackId) {
        //i++;  if (i>20)  break;

        const uint32_t moduleId(kv.first);
        const StackedTrackerDetId stackDetId(kv.second);
        const PixelGeomDetUnit* pixUnit0 = dynamic_cast<const PixelGeomDetUnit*>(theStackedGeometry->idToDetUnit(stackDetId, 0));
        const PixelGeomDetUnit* pixUnit1 = dynamic_cast<const PixelGeomDetUnit*>(theStackedGeometry->idToDetUnit(stackDetId, 1));
        const Surface::PositionType& surfposition0 = pixUnit0->position();
        const Surface::PositionType& surfposition1 = pixUnit1->position();
        const float separation = (moduleId < 110000) ? surfposition1.perp() - surfposition0.perp() : ((moduleId < 180000) ? surfposition1.z() - surfposition0.z() : surfposition0.z() - surfposition1.z());

        //std::cout << moduleId << ", " << surfposition0.x() << ", " << surfposition0.y() << ", " << surfposition0.z() << ", " << surfposition0.perp() << ", " << surfposition0.eta() << ", " << surfposition0.phi() << std::endl;
        //std::cout << moduleId << ", " << surfposition1.x() << ", " << surfposition1.y() << ", " << surfposition1.z() << ", " << surfposition1.perp() << ", " << surfposition1.eta() << ", " << surfposition1.phi() << std::endl;

        // Positions are in unit of centimeter
        csvfile.unsetf(std::ios_base::floatfield);
        csvfile << moduleId << ", "
                << std::fixed << std::setprecision(6)
                << surfposition0.z() << ", "
                << surfposition0.perp() << ", "
                << surfposition0.eta() << ", "
                << surfposition0.phi() << ", "
                << separation
                << std::endl;
    }

    // Close text file
    csvfile.close();

    std::cout << ">>> " << csvfile_ << " is written." << std::endl;
}


// ANALYZE
void AnalyzerModuleCoordinates::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // Do nothing
}

// DEFINE THIS AS A PLUG-IN
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(AnalyzerModuleCoordinates);
