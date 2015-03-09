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
#include "DataFormats/SiPixelDetId/interface/PXBDetId.h"
#include "DataFormats/SiPixelDetId/interface/PXFDetId.h"
#include "DataFormats/SiPixelDetId/interface/StackedTrackerDetId.h"
//#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
//#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "TString.h"


class AnalyzerModuleVertices : public edm::EDAnalyzer {
  public:
    /// Constructor/destructor
    explicit AnalyzerModuleVertices(const edm::ParameterSet&);
    virtual ~AnalyzerModuleVertices();

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


AnalyzerModuleVertices::AnalyzerModuleVertices(const edm::ParameterSet& iConfig)
: csvfile_(iConfig.getParameter<std::string>("csv") ),
  verbose_(iConfig.getParameter<int>("verbosity") ) {}

AnalyzerModuleVertices::~AnalyzerModuleVertices() {}


namespace {
uint32_t getModuleLayer(const DetId& id) {
    if (id.subdetId() == (int) PixelSubdetector::PixelBarrel) {
        PXBDetId pxbId(id);
        uint32_t layer  = pxbId.layer();
        return layer;
    } else if (id.subdetId() == (int) PixelSubdetector::PixelEndcap) {
        PXFDetId pxfId(id);
        uint32_t side   = pxfId.side();
        uint32_t disk   = pxfId.disk();
        uint32_t layer  = (side == 2) ? disk : disk+7;
        return layer;
    } else {  // error
        return 999999;
    }
}

uint32_t getModuleLadder(const DetId& id) {
    if (id.subdetId() == (int) PixelSubdetector::PixelBarrel) {
        PXBDetId pxbId(id);
        uint32_t ladder = pxbId.ladder();
        return ladder;
    } else if (id.subdetId() == (int) PixelSubdetector::PixelEndcap) {
        PXFDetId pxfId(id);
        uint32_t ring   = pxfId.ring();
        uint32_t blade  = pxfId.blade();
        uint32_t panel  = pxfId.panel();
        if(panel != 1 || ring != blade) {  // error
            return 999999;
        }
        return ring;
    } else {  // error
        return 999999;
    }
}

uint32_t getModuleModule(const DetId& id) {
    if (id.subdetId() == (int) PixelSubdetector::PixelBarrel) {
        PXBDetId pxbId(id);
        uint32_t module = pxbId.module();
        return module;
    } else if (id.subdetId() == (int) PixelSubdetector::PixelEndcap) {
        PXFDetId pxfId(id);
        uint32_t module = pxfId.module();
        return module;
    } else {  // error
        return 999999;
    }
}

uint32_t getModuleId(const DetId& id) {
    uint32_t layer  = getModuleLayer(id);
    uint32_t ladder = getModuleLadder(id);
    uint32_t module = getModuleModule(id);
    assert(layer != 999999 && ladder != 999999 && module != 999999);
    return 10000*layer + 100*(ladder-1) + (module-1)/2;
}
}


// Here we make the layout of the detector
void AnalyzerModuleVertices::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    /// Geometry setup
    /// Set pointers to Geometry
    edm::ESHandle<TrackerGeometry> geometryHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geometryHandle);
    theGeometry = geometryHandle.product();

    /// Set pointers to Stacked Modules
    edm::ESHandle<StackedTrackerGeometry> stackedGeometryHandle;
    iSetup.get<StackedTrackerGeometryRecord>().get(stackedGeometryHandle);
    theStackedGeometry = stackedGeometryHandle.product();

    /// Clear maps
    moduleId0ToStackId.clear();
    moduleId1ToStackId.clear();
    moduleId0ToGeoId.clear();
    moduleId1ToGeoId.clear();

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
        uint32_t moduleId1 = getModuleId(geoId0);
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

void AnalyzerModuleVertices::endRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {}


void AnalyzerModuleVertices::beginJob() {}


// Here we write into the text file
void AnalyzerModuleVertices::endJob() {
    std::cout << "Map size: " << moduleId0ToStackId.size() << ", " << moduleId1ToStackId.size()
              << ", " << moduleId0ToGeoId.size() << ", " << moduleId1ToGeoId.size() << std::endl;

    // Open text file
    ofstream csvfile(csvfile_);

    // Write text file
    //int i=0;
    csvfile << "moduleId/I, x0_cm/D, y0_cm/D, z0_cm/D, x1_cm/D, y1_cm/D, z1_cm/D, x2_cm/D, y2_cm/D, z2_cm/D, x3_cm/D, y3_cm/D, z3_cm/D" << std::endl;

    for (const auto& kv : moduleId0ToStackId) {
        //i++;  if (i>20)  break;

        const uint32_t moduleId(kv.first);
        const StackedTrackerDetId stackDetId(kv.second);

        for (unsigned i=0; i<2; ++i) {
            const PixelGeomDetUnit* pixUnit = dynamic_cast<const PixelGeomDetUnit*>(theStackedGeometry->idToDetUnit(stackDetId, i));
            const PixelTopology* pixTopo = dynamic_cast<const PixelTopology*>(&(pixUnit->specificTopology()) );

            int nrows = pixTopo->nrows();
            int ncols = pixTopo->ncolumns();

            /// Add 0.5 to get the center of the pixel
            MeasurementPoint mp0(0    , 0    );
            MeasurementPoint mp1(nrows, 0    );
            MeasurementPoint mp2(nrows, ncols);
            MeasurementPoint mp3(0    , ncols);

            /// Find global positions
            const GlobalPoint& pos0 = pixUnit->surface().toGlobal(pixUnit->topology().localPosition(mp0));
            const GlobalPoint& pos1 = pixUnit->surface().toGlobal(pixUnit->topology().localPosition(mp1));
            const GlobalPoint& pos2 = pixUnit->surface().toGlobal(pixUnit->topology().localPosition(mp2));
            const GlobalPoint& pos3 = pixUnit->surface().toGlobal(pixUnit->topology().localPosition(mp3));

            // Positions are in unit of centimeter
            csvfile.unsetf(std::ios_base::floatfield);
            csvfile << moduleId << ", "
                    << std::fixed << std::setprecision(6)
                    << pos0.x() << ", " << pos0.y() << ", " << pos0.z() << ", "
                    << pos1.x() << ", " << pos1.y() << ", " << pos1.z() << ", "
                    << pos2.x() << ", " << pos2.y() << ", " << pos2.z() << ", "
                    << pos3.x() << ", " << pos3.y() << ", " << pos3.z()
                    << std::endl;
        }
    }

    // Close text file
    csvfile.close();

    std::cout << ">>> " << csvfile_ << " is written." << std::endl;
}


// ANALYZE
void AnalyzerModuleVertices::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // Do nothing
}

// DEFINE THIS AS A PLUG-IN
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(AnalyzerModuleVertices);
