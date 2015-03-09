#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TriggerTowerMap.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/SuperstripArbiter.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Helper.h"
using namespace slhcl1tt;

#include <cppunit/extensions/HelperMacros.h>
#include <stdexcept>


// _____________________________________________________________________________
// Test fixedwidth superstrip
unsigned test_ss256_nz2_tt27(unsigned moduleId, unsigned moduleCode, float strip, float segment) {
    unsigned ss = unsigned(strip) / 256;

    if (isPSModule(moduleId))
        ss |= ((unsigned(segment) / 16) << 2);
    else
        ss |= ((unsigned(segment*16) / 16) << 2);

    ss |= (moduleCode << (2+1));
    return ss;
}

unsigned test_ss1_nz32_tt27(unsigned moduleId, unsigned moduleCode, float strip, float segment) {
    unsigned ss = unsigned(strip) / 1;

    if (isPSModule(moduleId))
        ss |= ((unsigned(segment) / 1) << 10);
    else
        ss |= ((unsigned(segment*16) / 1) << 10);

    ss |= (moduleCode << (10+5));
    return ss;
}

unsigned test_ss1024_nz1_tt27(unsigned moduleId, unsigned moduleCode, float strip, float segment) {
    unsigned ss = unsigned(strip) / 1024;

    if (isPSModule(moduleId))
        ss |= ((unsigned(segment) / 32) << 0);
    else
        ss |= ((unsigned(segment*16) / 32) << 0);

    ss |= (moduleCode << 0);
    return ss;
}

// _____________________________________________________________________________
// Test projective superstrip (from the old version)
unsigned superstrip_projective(unsigned lay, float phi, float z,
                               const float unit_phi, const float divide_z) {
    unsigned h = 0;

    static const float edges_phi[6*2] = {0.564430,1.791765,0.653554,1.710419,0.641981,1.756567,0.717273,1.638922,0.658179,1.673851,0.618448,1.778293};
    static const float edges_z[6*2] = {-6.7127,26.9799,-6.7797,36.7048,-5.2542,47.7511,-9.5318,59.4103,-9.5318,78.7372,-9.5318,88.9935};
    float unit_z   = (edges_z[2*5+1] - edges_z[2*5]) / divide_z;  // outermost has the largest unit

    int n_phi = floor(M_PI / 4. / unit_phi + 0.5);
    int n_z   = divide_z;
    assert(n_phi > 0 && n_z > 0);

    if (lay < 16) {
        unit_z   = (edges_z[2*lay+1] - edges_z[2*lay]) / divide_z;

        phi -= edges_phi[2*lay];  // lowest phi value in trigger tower
        z   -= edges_z[2*lay];    // lowest z value in trigger tower

        int i_phi = floor(phi / unit_phi);
        int i_z   = floor(z   / unit_z);
        i_phi = (i_phi < 0) ? 0 : (i_phi >= n_phi) ? (n_phi - 1) : i_phi;  // proper range
        i_z   = (i_z   < 0) ? 0 : (i_z   >= n_z  ) ? (n_z   - 1) : i_z  ;  // proper range

        h = i_z * n_phi + i_phi;
    }

    return h;
}

// _____________________________________________________________________________
// Test fountain superstrip (from the old version)
unsigned superstrip_fountain(unsigned lay, float phi, float z,
                             const float scale_phi, const float divide_z) {
    unsigned h = 0;

    static const float units_phi[6] = {0.00381, 0.00439, 0.00459, 0.00485, 0.00523, 0.00575};
    static const float edges_phi[6*2] = {0.564430,1.791765,0.653554,1.710419,0.641981,1.756567,0.717273,1.638922,0.658179,1.673851,0.618448,1.778293};
    static const float edges_z[6*2] = {-6.7127,26.9799,-6.7797,36.7048,-5.2542,47.7511,-9.5318,59.4103,-9.5318,78.7372,-9.5318,88.9935};

    float unit_phi = units_phi[0] * scale_phi;                    // innermost has the smallest unit
    float unit_z   = (edges_z[2*5+1] - edges_z[2*5]) / divide_z;  // outermost has the largest unit
    //if (unit_phi > M_PI*2.)  unit_phi = M_PI*2.;
    //if (unit_z   > 2.2*2. )  unit_z   = 2.2*2.;
    int n_phi = floor((edges_phi[2*lay+1] - edges_phi[2*lay]) / unit_phi + 0.5);
    int n_z   = divide_z;
    assert(n_phi > 0 && n_z > 0);

    if (lay < 6) {  // barrel only
        unit_phi = units_phi[lay] * scale_phi;
        unit_z   = (edges_z[2*lay+1] - edges_z[2*lay]) / divide_z;
        //if (unit_phi > M_PI*2.)  unit_phi = M_PI*2.;
        //if (unit_z   > 2.2*2. )  unit_z   = 2.2*2.;

        phi -= edges_phi[2*lay];  // lowest phi value in trigger tower
        z   -= edges_z[2*lay];    // lowest z value in trigger tower

        int i_phi = floor(phi / unit_phi);
        int i_z   = floor(z   / unit_z);
        i_phi = (i_phi < 0) ? 0 : (i_phi >= n_phi) ? (n_phi - 1) : i_phi;  // proper range
        i_z   = (i_z   < 0) ? 0 : (i_z   >= n_z  ) ? (n_z   - 1) : i_z  ;  // proper range

        h = i_z * n_phi + i_phi;
    }

    return h;
}


// _____________________________________________________________________________
// Unit test class
class TestSuperstripOperations : public CppUnit::TestFixture  {

CPPUNIT_TEST_SUITE(TestSuperstripOperations);
CPPUNIT_TEST(testArbiterDefinition);
CPPUNIT_TEST(testArbiterFixedwidth);
CPPUNIT_TEST(testArbiterProjective);
CPPUNIT_TEST(testArbiterFountain);
CPPUNIT_TEST_SUITE_END();

private:
    TriggerTowerMap   * ttmap_;
    SuperstripArbiter * arbiter_;

    unsigned                 tt_;
    std::map<unsigned, bool> ttrmap_;

public:
    void setUp() {
        ttmap_   = new TriggerTowerMap();
        arbiter_ = new SuperstripArbiter();


        TString datadir = std::getenv("CMSSW_BASE");
        datadir += "/src/SLHCL1TrackTriggerSimulations/AMSimulation/data/";
        TString csvfile1 = datadir + "trigger_sector_map.csv";
        TString csvfile2 = datadir + "trigger_sector_boundaries.csv";
        ttmap_ -> readTriggerTowerMap(csvfile1);
        ttmap_ -> readTriggerTowerBoundaries(csvfile2);

        tt_ = 27;
        ttrmap_ = ttmap_ -> getTriggerTowerReverseMap(tt_);
    }

    void tearDown() {
        if (ttmap_)     delete ttmap_;
        if (arbiter_)   delete arbiter_;
    }

    void testArbiterDefinition() {
        TString ss = "ss0_nz32";
        CPPUNIT_ASSERT_THROW(arbiter_ -> setDefinition(ss, tt_, ttmap_), std::invalid_argument);

        ss = "ss1_nz0";
        CPPUNIT_ASSERT_THROW(arbiter_ -> setDefinition(ss, tt_, ttmap_), std::invalid_argument);

        ss = "ss1025_nz32";
        CPPUNIT_ASSERT_THROW(arbiter_ -> setDefinition(ss, tt_, ttmap_), std::invalid_argument);

        ss = "ss1_nz33";
        CPPUNIT_ASSERT_THROW(arbiter_ -> setDefinition(ss, tt_, ttmap_), std::invalid_argument);

        ss = "ss3_nz2";
        CPPUNIT_ASSERT_THROW(arbiter_ -> setDefinition(ss, tt_, ttmap_), std::invalid_argument);

        ss = "ss256_nz3";
        CPPUNIT_ASSERT_THROW(arbiter_ -> setDefinition(ss, tt_, ttmap_), std::invalid_argument);
    }

    void testArbiterFixedwidth() {
        if (true) {
            TString ss = "ss256_nz2";
            arbiter_ -> setDefinition(ss, tt_, ttmap_);
            arbiter_ -> print();

            unsigned lay = 5;
            unsigned i = 0;
            for (auto itmap : ttrmap_) {
                unsigned moduleId = itmap.first;
                if (lay != decodeLayer(moduleId)) {  // new layer
                    ++lay;
                    i = 0;
                }

                for (unsigned j=0; j<32; ++j) {
                    if (!isPSModule(moduleId) && j == 2)  // for 2S modules, j=0,1
                        break;
                    float segment = j;

                    for (unsigned k=0; k<1024; ++k) {
                        float strip = k;

                        unsigned ss = arbiter_ -> superstripLocal(moduleId, strip, segment);
                        unsigned ss256 = test_ss256_nz2_tt27(moduleId, i, strip, segment);
                        //std::cout << moduleId << " " << i << " " << strip << " " << segment << std::endl;

                        CPPUNIT_ASSERT_EQUAL(ss256, ss);
                    }
                }
                ++i;
            }
        }

        if (true) {
            TString ss = "ss1_nz32";
            arbiter_ -> setDefinition(ss, tt_, ttmap_);
            arbiter_ -> print();

            unsigned lay = 5;
            unsigned i = 0;
            for (auto itmap : ttrmap_) {
                unsigned moduleId = itmap.first;
                if (lay != decodeLayer(moduleId)) {  // new layer
                    ++lay;
                    i = 0;
                }

                for (unsigned j=0; j<32; ++j) {
                    if (!isPSModule(moduleId) && j == 2)  // for 2S modules, j=0,1
                        break;
                    float segment = j;

                    for (unsigned k=0; k<1024; ++k) {
                        float strip = k;

                        unsigned ss = arbiter_ -> superstripLocal(moduleId, strip, segment);
                        unsigned ss1 = test_ss1_nz32_tt27(moduleId, i, strip, segment);
                        //std::cout << moduleId << " " << i << " " << strip << " " << segment << std::endl;

                        CPPUNIT_ASSERT_EQUAL(ss1, ss);
                    }
                }
                ++i;
            }
        }

        if (false) {
            TString ss = "ss1024_nz1";
            arbiter_ -> setDefinition(ss, tt_, ttmap_);
            arbiter_ -> print();

            unsigned lay = 5;
            unsigned i = 0;
            for (auto itmap : ttrmap_) {
                unsigned moduleId = itmap.first;
                if (lay != decodeLayer(moduleId)) {  // new layer
                    ++lay;
                    i = 0;
                }

                for (unsigned j=0; j<32; ++j) {
                    if (!isPSModule(moduleId) && j == 2)  // for 2S modules, j=0,1
                        break;
                    float segment = j;

                    for (unsigned k=0; k<1024; ++k) {
                        float strip = k;

                        unsigned ss = arbiter_ -> superstripLocal(moduleId, strip, segment);
                        unsigned ss1024 = test_ss1024_nz1_tt27(moduleId, i, strip, segment);
                        //std::cout << moduleId << " " << i << " " << strip << " " << segment << std::endl;

                        CPPUNIT_ASSERT_EQUAL(ss1024, ss);
                    }
                }
                ++i;
            }
        }
    }

    void testArbiterProjective() {
        static const float edges_phi[6*2] = {0.564430,1.791765,0.653554,1.710419,0.641981,1.756567,0.717273,1.638922,0.658179,1.673851,0.618448,1.778293};
        static const float edges_z[6*2] = {-6.7127,26.9799,-6.7797,36.7048,-5.2542,47.7511,-9.5318,59.4103,-9.5318,78.7372,-9.5318,88.9935};

        if (true) {
            TString ss = "nx200_nz1";
            arbiter_ -> setDefinition(ss, tt_, ttmap_);
            arbiter_ -> print();

            for (unsigned i=0; i<6; ++i) {
                unsigned moduleId = (5+i) * 10000;
                float r = 20.;  // dummy
                float ds = 0.;  // dummy

                for (unsigned j=0; j<101; ++j) {
                    float phi = edges_phi[i*2] + (edges_phi[i*2+1] - edges_phi[i*2]) / 100. * j;

                    for (unsigned k=0; k<101; ++k) {
                        float z = edges_z[i*2] + (edges_z[i*2+1] - edges_z[i*2]) / 100 * k;

                        unsigned ss = arbiter_ -> superstripGlobal(moduleId, r, phi, z, ds);
                        unsigned ss200 = superstrip_projective(i, phi, z, M_PI / 4. / 200., 1.);
                        //std::cout << moduleId << " " << r << " " << phi << " " << z << " " << ds << std::endl;

                        CPPUNIT_ASSERT_EQUAL(ss200, ss);
                    }
                }
            }
        }

        if (true) {
            TString ss = "nx400_nz2";
            arbiter_ -> setDefinition(ss, tt_, ttmap_);
            arbiter_ -> print();

            for (unsigned i=0; i<6; ++i) {
                unsigned moduleId = (5+i) * 10000;
                float r = 20.;  // dummy
                float ds = 0.;  // dummy

                for (unsigned j=0; j<101; ++j) {
                    float phi = edges_phi[i*2] + (edges_phi[i*2+1] - edges_phi[i*2]) / 100. * j;

                    for (unsigned k=0; k<101; ++k) {
                        float z = edges_z[i*2] + (edges_z[i*2+1] - edges_z[i*2]) / 100 * k;

                        unsigned ss = arbiter_ -> superstripGlobal(moduleId, r, phi, z, ds);
                        unsigned ss400 = superstrip_projective(i, phi, z, M_PI / 4. / 400., 2.);
                        //std::cout << moduleId << " " << r << " " << phi << " " << z << " " << ds << std::endl;

                        CPPUNIT_ASSERT_EQUAL(ss400, ss);
                    }
                }
            }
        }
    }

    void testArbiterFountain() {
        static const float edges_phi[6*2] = {0.564430,1.791765,0.653554,1.710419,0.641981,1.756567,0.717273,1.638922,0.658179,1.673851,0.618448,1.778293};
        static const float edges_z[6*2] = {-6.7127,26.9799,-6.7797,36.7048,-5.2542,47.7511,-9.5318,59.4103,-9.5318,78.7372,-9.5318,88.9935};

        if (true) {
            TString ss = "sf1_nz1";
            arbiter_ -> setDefinition(ss, tt_, ttmap_);
            arbiter_ -> print();

            for (unsigned i=0; i<6; ++i) {
                unsigned moduleId = (5+i) * 10000;
                float r = 20.;  // dummy
                float ds = 0.;  // dummy

                for (unsigned j=0; j<101; ++j) {
                    float phi = edges_phi[i*2] + (edges_phi[i*2+1] - edges_phi[i*2]) / 100. * j;

                    for (unsigned k=0; k<101; ++k) {
                        float z = edges_z[i*2] + (edges_z[i*2+1] - edges_z[i*2]) / 100 * k;

                        unsigned ss = arbiter_ -> superstripGlobal(moduleId, r, phi, z, ds);
                        unsigned ss1 = superstrip_fountain(i, phi, z, 1., 1.);
                        //std::cout << moduleId << " " << r << " " << phi << " " << z << " " << ds << std::endl;

                        CPPUNIT_ASSERT_EQUAL(ss1, ss);
                    }
                }
            }
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestSuperstripOperations);
