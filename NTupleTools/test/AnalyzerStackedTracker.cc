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
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
#include "Geometry/CommonTopologies/interface/PixelTopology.h"

#include "SLHCL1TrackTriggerSimulations/NTupleTools/interface/ModuleIdFunctor.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <limits>
//#include "TString.h"


template<class T>
bool almost_equal(T x, T y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y) < std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
    // unless the result is subnormal
           || std::abs(x-y) < std::numeric_limits<T>::min();
}


class AnalyzerStackedTracker : public edm::EDAnalyzer {
  public:
    /// Constructor/destructor
    explicit AnalyzerStackedTracker(const edm::ParameterSet&);
    virtual ~AnalyzerStackedTracker();

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

    /// Configurations
    std::string csvfile_;
    int verbose_;
};


AnalyzerStackedTracker::AnalyzerStackedTracker(const edm::ParameterSet& iConfig)
: csvfile_(iConfig.getParameter<std::string>("csv") ),
  verbose_(iConfig.getParameter<int>("verbosity") ) {}

AnalyzerStackedTracker::~AnalyzerStackedTracker() {}


// Here we make the layout of the detector
void AnalyzerStackedTracker::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
    /// Geometry setup
    edm::ESHandle<TrackerGeometry> geometryHandle;
    iSetup.get<TrackerDigiGeometryRecord>().get(geometryHandle);
    theGeometry = geometryHandle.product();

    edm::ESHandle<StackedTrackerGeometry> stackedGeometryHandle;
    iSetup.get<StackedTrackerGeometryRecord>().get(stackedGeometryHandle);
    theStackedGeometry = stackedGeometryHandle.product();

    assert(theStackedGeometry->getCBC3MaxStubs() == 0);
    assert(theStackedGeometry->getPartitionsPerRoc() == 0);
    assert(theStackedGeometry->stacks().size() == 15428);

    /// Prepare detId -> moduleId
    ModuleIdFunctor getModuleId;

    /// Loop over the detector elements
    StackedTrackerGeometry::StackContainerIterator  stkIterator;
    for (stkIterator = theStackedGeometry->stacks().begin();
         stkIterator != theStackedGeometry->stacks().end();
         ++stkIterator) {

        StackedTrackerDetUnit* stackDetUnit = *stkIterator;
        StackedTrackerDetId stackDetId(stackDetUnit->Id());
        assert(stackDetUnit == theStackedGeometry->idToStack(stackDetId));

        bool isBarrel = stackDetId.isBarrel();
        bool isEndcap = stackDetId.isEndcap();
        bool isPSModule = theStackedGeometry->isPSModule(stackDetId);

        // 0 means inner, 1 means outer
        const GeomDet* det0 = theStackedGeometry->idToDet(stackDetId, 0);
        const GeomDet* det1 = theStackedGeometry->idToDet(stackDetId, 1);
        const DetId geoId0 = det0->geographicalId();
        const DetId geoId1 = det1->geographicalId();

        const unsigned moduleId0 = getModuleId(geoId0);
        const unsigned moduleId1 = getModuleId(geoId1);

        const GeomDetUnit* detUnit0 = theStackedGeometry->idToDetUnit(stackDetId, 0);
        const GeomDetUnit* detUnit1 = theStackedGeometry->idToDetUnit(stackDetId, 1);
        const PixelGeomDetUnit* pixUnit0 = dynamic_cast<const PixelGeomDetUnit*>(detUnit0);
        const PixelGeomDetUnit* pixUnit1 = dynamic_cast<const PixelGeomDetUnit*>(detUnit1);

        const PixelTopology* pixTopo0 = &(pixUnit0->specificTopology());
        const PixelTopology* pixTopo1 = &(pixUnit1->specificTopology());
        const PixelGeomDetType* pixType0 = &(pixUnit0->specificType());
        const PixelGeomDetType* pixType1 = &(pixUnit1->specificType());

        assert(isBarrel || isEndcap);
        assert(moduleId0 == moduleId1);
        assert(pixType0->isTrackerPixel() && pixType1->isTrackerPixel());

        // Check layer, ladder, module, etc
        if (isBarrel) {
            //std::cout << "Barrel " << moduleId0 << std::endl;
            //std::cout << stackDetId.iLayer() << " " << stackDetId.iPhi() << " " << stackDetId.iZ() << std::endl;

            unsigned moduleIdTest = (stackDetId.iLayer() - 1 + 5) * 10000
                                  + (stackDetId.iPhi() - 1) * 100
                                  + (stackDetId.iZ() - 1);
            assert(moduleId0 == moduleIdTest);
        } else {
            //std::cout << "Endcap " << moduleId0 << std::endl;
            //std::cout << stackDetId.iSide() << " " << stackDetId.iZ() << " " << stackDetId.iRing() << " " << stackDetId.iPhi() << std::endl;

            unsigned moduleIdTest = (stackDetId.iZ() - 1 + 11) * 10000
                                  + (stackDetId.iRing() - 1) * 100
                                  + (stackDetId.iPhi() - 1);
            if (stackDetId.iSide() == 1)
                moduleIdTest += 7 * 10000;
            assert(moduleId0 == moduleIdTest);
        }


        // Constants
        // See Geometry/TrackerGeometryBuilder/interface/RectangularPixelTopology.h
        // See Geometry/TrackerGeometryBuilder/interface/PixelTopologyBuilder.h
        uint32_t ncolumns0    = pixTopo0->ncolumns();
        uint32_t nrows0       = pixTopo0->nrows();
        uint32_t pitchX0      = pixTopo0->pitch().first;  // = width / nrows
        uint32_t pitchY0      = pixTopo0->pitch().second; // = length / ncols
        uint32_t rocsX0       = pixTopo0->rocsX();
        uint32_t rocsY0       = pixTopo0->rocsY();
        uint32_t colsperroc0  = pixTopo0->colsperroc();  // = ncolumns / rocsY
        uint32_t rowsperroc0  = pixTopo0->rowsperroc();  // = nrows / rocsX
        const Bounds& bounds0 = pixUnit0->specificSurface().bounds();
        float detThickness0   = bounds0.thickness();
        float detLength0      = bounds0.length();
        float detWidth0       = bounds0.width();

        uint32_t ncolumns1    = pixTopo1->ncolumns();
        uint32_t nrows1       = pixTopo1->nrows();
        uint32_t pitchX1      = pixTopo1->pitch().first;  // = width / nrows
        uint32_t pitchY1      = pixTopo1->pitch().second; // = length / ncols
        uint32_t rocsX1       = pixTopo1->rocsX();
        uint32_t rocsY1       = pixTopo1->rocsY();
        uint32_t colsperroc1  = pixTopo1->colsperroc();  // = ncolumns / rocsY
        uint32_t rowsperroc1  = pixTopo1->rowsperroc();  // = nrows / rocsX
        const Bounds& bounds1 = pixUnit1->specificSurface().bounds();
        float detThickness1   = bounds1.thickness();
        float detLength1      = bounds1.length();
        float detWidth1       = bounds1.width();

        if (isPSModule) {
            //std::cout << "PSModule " << moduleId0 << std::endl;
            //std::cout << detThickness0 << " " << detLength0 << " " << detWidth0 << std::endl;
            //std::cout << detThickness1 << " " << detLength1 << " " << detWidth1 << std::endl;

            assert(ncolumns0 == 32 && nrows0 == 960);
            assert(pitchX0 == 0 && pitchY0 == 0);
            assert(rocsX0 == 4 && rocsY0 == 2);
            assert(colsperroc0 == 16 && rowsperroc0 == 240);
            assert(almost_equal(detThickness0, 0.02f, 2) && almost_equal(detLength0, 4.626f, 2) && almost_equal(detWidth0, 9.6f, 2));

            assert(ncolumns1 == 2 && nrows1 == 960);
            assert(pitchX1 == 0 && pitchY1 == 2);
            assert(rocsX1 == 4 && rocsY1 == 2);
            assert(colsperroc1 == 1 && rowsperroc1 == 240);
            assert(almost_equal(detThickness1, 0.02f, 2) && almost_equal(detLength1, 4.626f, 2) && almost_equal(detWidth1, 9.6f, 2));

        } else {
            //std::cout << "2SModule " << moduleId0 << std::endl;
            //std::cout << detThickness0 << " " << detLength0 << " " << detWidth0 << std::endl;
            //std::cout << detThickness1 << " " << detLength1 << " " << detWidth1 << std::endl;

            assert(ncolumns0 == 2 && nrows0 == 1016);
            assert(pitchX0 == 0 && pitchY0 == 5);
            assert(rocsX0 == 8 && rocsY0 == 2);
            assert(colsperroc0 == 1 && rowsperroc0 == 127);
            assert(almost_equal(detThickness0, 0.02f, 2) && almost_equal(detLength0, 10.05f, 2) && almost_equal(detWidth0, 9.144f, 2));

            assert(ncolumns1 == 2 && nrows1 == 1016);
            assert(pitchX1 == 0 && pitchY1 == 5);
            assert(rocsX1 == 8 && rocsY1 == 2);
            assert(colsperroc1 == 1 && rowsperroc1 == 127);
            assert(almost_equal(detThickness1, 0.02f, 2) && almost_equal(detLength1, 10.05f, 2) && almost_equal(detWidth1, 9.144f, 2));

        }

        //std::cout << theStackedGeometry->getDetUnitWindow(stackDetId) << std::endl;

    }  // end loop over stacks

}

void AnalyzerStackedTracker::endRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {}


void AnalyzerStackedTracker::beginJob() {}


// Here we write into the text file
void AnalyzerStackedTracker::endJob() {
    // Open text file
    ofstream csvfile(csvfile_);

    // Write text file
    // Write nothing

    // Close text file
    csvfile.close();

    std::cout << ">>> " << csvfile_ << " is written." << std::endl;
}


// ANALYZE
void AnalyzerStackedTracker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
    // Do nothing
}

// DEFINE THIS AS A PLUG-IN
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(AnalyzerStackedTracker);
