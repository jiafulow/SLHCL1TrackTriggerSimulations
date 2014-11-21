import unittest
import sys
from ROOT import TFile, TTree, gROOT, gSystem, vector


class TestAMSim(unittest.TestCase):
    """Test the pattern recognition results using PatternMatcher"""

    infile = "roads.root"

    def setUp(self):
        gROOT.SetBatch(True)
        gROOT.SetMacroPath(gSystem.Getenv("CMSSW_BASE")+"/src/SLHCL1TrackTriggerSimulations/AMSimulation")
        gROOT.LoadMacro("python/test/loader.h+")
        self.tfile = TFile.Open(self.infile)
        self.ttree = self.tfile.Get("ntupler/tree")
        self.nevents = self.ttree.GetEntries()

    def tearDown(self):
        pass

    def test_nevents(self):
        self.assertEqual(self.nevents, 100)

    def test_nroads(self):
        tree = self.ttree
        n = 0
        for ievt in tree:
            for iroad in ievt.AMTTRoads_stubRefs:
                n += 1
            self.assertTrue(ievt.AMTTRoads_stubRefs.size() in [0, 1, 7, 28])
        #self.assertEqual(n, 63 + 15 * 7 + 0 * 28)
        #self.assertEqual(n, 78)
        self.assertEqual(n, 77)

    def test_nhits(self):
        tree = self.ttree
        n = 0
        for ievt in tree:
            for iroad in ievt.AMTTRoads_stubRefs:
                for ihit in iroad:
                    n += 1
                self.assertTrue(iroad.size() in [0, 6, 7, 8])
        #self.assertEqual(n, 1031)
        #self.assertEqual(n, 476)
        self.assertEqual(n, 471)

    def test_nsuperstrips(self):
        tree = self.ttree
        for ievt in tree:
            for iroad in ievt.AMTTRoads_nsuperstrips:
                self.assertEqual(iroad, 6)

    def test_bankIndex(self):
        tree = self.ttree
        bankIndices = {}
        for ievt in tree:
            for iroad in ievt.AMTTRoads_bankIndex:
                self.assertTrue(iroad not in bankIndices)
                bankIndices[iroad] = 0


if __name__ == "__main__":
    if len(sys.argv) > 1:
        TestAMSim.infile = sys.argv.pop()

    unittest.main()
