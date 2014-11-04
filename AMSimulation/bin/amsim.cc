#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/StubCleaner.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternGenerator.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/PatternMatcher.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/TrackFitter.h"
#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/NTupleMaker.h"

#include "boost/program_options.hpp"
#include <cstdlib>
#include <fstream>

// To print default vectors
namespace boost {
    template<class T>
    std::ostream& operator<<(std::ostream& o, const std::vector<T>& v) {
        std::copy(v.begin(), v.end(), std::ostream_iterator<T>(o, " "));
        return o;
    }
}


// _____________________________________________________________________________
int main(int argc, char **argv) {

    std::cout << "amsim arguments << ";
    for(int i = 1; i < argc-1; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << argv[argc-1] << std::endl << std::endl;

    // Declare a group of options that will be allowed only on command line
    namespace po = boost::program_options;
    int verbose;
    po::options_description generic("Generic options");
    generic.add_options()
        ("version"             , "Print version")
        ("help,h"              , "Produce help message")
        ("cleanStubs,C"        , "Clean stubs and pick one unique stub per layer")
        ("generateBank,G"      , "Generate associative memory pattern bank")
        ("mergeBank,M"         , "Merge associative memory pattern bank")
        ("analyzeBank,A"       , "Analyze associative memory pattern bank")
        ("patternRecognition,R", "Run associative memory pattern recognition")
        ("trackFitting,F"      , "Perform track fitting")
        ("write,W"             , "Write full ntuple")
        ("verbosity,v"         , po::value<int>(&verbose)->default_value(1), "Verbosity level (-1 = very quiet; 0 = quiet, 1 = verbose, 2+ = debug)")
        ("no-color"            , "Turn off colored text")
        ("timing"              , "Show timing information")
        ;

    // Declare a group of options that will be allowed both on command line
    // and in config file
    std::string input, output, layout, bankfile, roadfile, trackfile;
    long long maxEvents;
    int minFrequency, maxPatterns, maxRoads, maxHits, maxTracks;
    bool nofilter, notrim;
    PatternBankOption bankOption;
    TrackFitterOption fitOption;
    unsigned dividePhi, divideZ;

    const std::vector<unsigned> dv_subLadderVarSize = {8, 16, 32, 16, 16, 32};
    const std::vector<unsigned> dv_subModuleVarSize = {256, 256, 256, 256, 512, 512};
    const std::vector<unsigned> dv_subLadderECVarSize = {8, 8, 8, 16, 16, 16, 16, 32, 32, 16, 16, 16, 16, 32, 32};
    const std::vector<unsigned> dv_subModuleECVarSize = {256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 512, 512, 512, 512};
    const std::vector<unsigned> dv_triggerTowers = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};

    po::options_description config("Configuration");
    config.add_options()
        ("input,i"      , po::value<std::string>(&input)->required(), "Specify input files")
        ("output,o"     , po::value<std::string>(&output)->required(), "Specify output file")
        ("maxEvents,n"  , po::value<long long>(&maxEvents)->default_value(-1), "Specfiy max number of events")

        // Only for stub filtering
        ("no-filter"    , po::bool_switch(&nofilter)->default_value(false), "Do not apply filtering")

        // Only for bank generation
        ("layout,L"     , po::value<std::string>(&layout), "Specify trigger tower layout file")
        ("minFrequency" , po::value<int>(&minFrequency)->default_value(1), "Specify min frequency of a pattern to be valid")

        // Only for pattern matching
        ("bank,B"       , po::value<std::string>(&bankfile), "Specify pattern bank file")
        ("maxPatterns"  , po::value<int>(&maxPatterns)->default_value(-1), "Specfiy max number of patterns")
        ("maxRoads"     , po::value<int>(&maxRoads)->default_value(-1), "Specfiy max number of roads per event")
        ("maxHits"      , po::value<int>(&maxHits)->default_value(-1), "Specfiy max number of hits per road")

        // Only for track fitting
        ("maxTracks"    , po::value<int>(&maxTracks)->default_value(-1), "Specfiy max number of tracks per event")

        // Only for writing full ntuple
        ("no-trim"      , po::bool_switch(&notrim)->default_value(false), "Do not trim TTree branches")
        ("roads"        , po::value<std::string>(&roadfile), "Specify file containing the roads")
        ("tracks"       , po::value<std::string>(&trackfile), "Specify file containing the tracks")

        // Specifically for a pattern bank
        ("bank_minPt"               , po::value<float>(&bankOption.minPt)->default_value(     2.0), "Specify min pt")
        ("bank_maxPt"               , po::value<float>(&bankOption.maxPt)->default_value(999999.0), "Specify max pt")
        ("bank_minEta"              , po::value<float>(&bankOption.minEta)->default_value(-2.5), "Specify min eta (signed)")
        ("bank_maxEta"              , po::value<float>(&bankOption.maxEta)->default_value( 2.5), "Specify max eta (signed)")
        ("bank_minPhi"              , po::value<float>(&bankOption.minPhi)->default_value(-M_PI), "Specify min phi (from -pi to pi)")
        ("bank_maxPhi"              , po::value<float>(&bankOption.maxPhi)->default_value( M_PI), "Specify max phi (from -pi to pi)")
        ("bank_dividePhi"           , po::value<unsigned>(&dividePhi)->default_value(400), "Specify the number of phi divisions in one trigger tower in simplified geometry")
        ("bank_divideZ"             , po::value<unsigned>(&divideZ)->default_value(1), "Specify the number of z divisions in one trigger tower in simplified geometry")
        ("bank_subLadderSize"       , po::value<unsigned>(&bankOption.subLadderSize)->default_value(4), "Specify the size of a subladder (a.k.a. segment)")
        ("bank_subModuleSize"       , po::value<unsigned>(&bankOption.subModuleSize)->default_value(8), "Specify the size of a submodule (a.k.a. superstrip)")
        ("bank_nLayers"             , po::value<unsigned>(&bankOption.nLayers)->default_value(6), "Specify # of layers")
        ("bank_nMisses"             , po::value<unsigned>(&bankOption.nMisses)->default_value(0), "Specify # of allowed misses")
        ("bank_nFakers"             , po::value<unsigned>(&bankOption.nFakers)->default_value(0), "Specify # of fake superstrips")
        ("bank_nDCBits"             , po::value<unsigned>(&bankOption.nDCBits)->default_value(0), "Specify # of DC bits")
        ("bank_mode"                , po::value<unsigned>(&bankOption.mode)->default_value(0), "Select superstrip config -- 0: Fixed-size; 1: Variable-size; 2: Projective")
        ("bank_requireTriggerTower" , po::value<bool>(&bankOption.requireTriggerTower)->default_value(false), "Apply trigger tower requirement")
        ("bank_subLadderVarSize"    , po::value<std::vector<unsigned> >(&bankOption.subLadderVarSize)->default_value(dv_subLadderVarSize), "Specify the variable size of a subladder per layer")
        ("bank_subModuleVarSize"    , po::value<std::vector<unsigned> >(&bankOption.subModuleVarSize)->default_value(dv_subModuleVarSize), "Specify the variable size of a submodule per layer")
        ("bank_subLadderECVarSize"  , po::value<std::vector<unsigned> >(&bankOption.subLadderECVarSize)->default_value(dv_subLadderECVarSize), "Specify the variable size of a subladder per endcap ring")
        ("bank_subModuleECVarSize"  , po::value<std::vector<unsigned> >(&bankOption.subModuleECVarSize)->default_value(dv_subModuleECVarSize), "Specify the variable size of a submodule per endcap ring")
        ("bank_triggerTowers"       , po::value<std::vector<unsigned> >(&bankOption.triggerTowers)->default_value(dv_triggerTowers), "Specify the trigger towers")

        // Specifically for a track fitter
        ("fit_pqType"               , po::value<int>(&fitOption.pqType)->default_value(0), "Specify choice of variables for p,q")
        ("fit_pbins"                , po::value<int>(&fitOption.pbins)->default_value(100), "Specify # of bins for p")
        ("fit_qbins"                , po::value<int>(&fitOption.qbins)->default_value(100), "Specify # of bins for q")
        ("fit_pmin"                 , po::value<float>(&fitOption.pmin)->default_value(-1), "Specify min value for p")
        ("fit_qmin"                 , po::value<float>(&fitOption.qmin)->default_value(-1), "Specify min value for q")
        ("fit_pmax"                 , po::value<float>(&fitOption.pmax)->default_value(1), "Specify max value for p")
        ("fit_qmax"                 , po::value<float>(&fitOption.qmax)->default_value(1), "Specify max value for q")
        ("fit_sigma"                , po::value<float>(&fitOption.sigma)->default_value(3), "Specify resolution for the distance parameter")
        ("fit_minWeight"            , po::value<float>(&fitOption.minWeight)->default_value(0.1), "Specify minimum weight to create a track")
        ;

    // Hidden options, will be allowed both on command line and in config file,
    // but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("config-file", po::value<std::vector<std::string> >(), "Specify config file")
        ;

    // Further group the options
    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description cfgfile_options;
    cfgfile_options.add(config).add(hidden);

    po::options_description visible("Allowed options");
    visible.add(generic).add(config);

    // Allow a positional option, which does not specify any name at all
    po::positional_options_description p;
    p.add("config-file", -1);

    // Parse program options
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
        if (vm.count("help") || argc==1) {
            std::cout << visible << std::endl;
            return EXIT_SUCCESS;
        }
        ifstream ifs("config-file");
        po::store(po::parse_config_file(ifs, cfgfile_options), vm);

        po::notify(vm);
    } catch (const boost::program_options::error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    // Options found in both PatternBankOption and TrackFitterOption
    fitOption.nLayers = bankOption.nLayers;

    // At least one option needs to be called
    if (!(vm.count("cleanStubs")         ||
          vm.count("generateBank")       ||
          vm.count("mergeBank")          ||
          vm.count("analyzeBank")        ||
          vm.count("patternRecognition") ||
          vm.count("trackFitting")       ||
          vm.count("write")              ) ) {
        std::cout << "Must select one of '-C', '-G', '-M', '-A', '-R', '-F' or 'W'" << std::endl;
        std::cout << visible << std::endl;
        return 1;
    }

    if (vm.count("no-color")) {
        slhcl1tt::NoColor();
    }

    if (vm.count("timing")) {
        slhcl1tt::ShowTiming();
    }

    // Restrict the interger ranges
    bankOption.subLadderSize    = std::min(std::max(1u, bankOption.subLadderSize), 32u);
    bankOption.subModuleSize    = std::min(std::max(2u, bankOption.subModuleSize), 1024u);
    bankOption.nLayers          = std::min(std::max(3u, bankOption.nLayers), 8u);
    bankOption.nMisses          = std::min(std::max(0u, bankOption.nMisses), 3u);
    bankOption.nFakers          = std::min(std::max(0u, bankOption.nFakers), 3u);
    bankOption.nDCBits          = std::min(std::max(0u, bankOption.nDCBits), 4u);
    bankOption.unitPhi          = (M_PI*2.0) / float(dividePhi * 8);
    bankOption.unitZ            = 640. / float(divideZ * 6);
    if (!bankOption.subLadderVarSize.empty())
        for (unsigned i=0; i<bankOption.subLadderVarSize.size(); ++i)
            bankOption.subLadderVarSize.at(i) = std::min(std::max(1u, bankOption.subLadderVarSize.at(i)), 32u);
    if (!bankOption.subModuleVarSize.empty())
        for (unsigned i=0; i<bankOption.subModuleVarSize.size(); ++i)
            bankOption.subModuleVarSize.at(i) = std::min(std::max(2u, bankOption.subModuleVarSize.at(i)), 1024u);
    if (!bankOption.subLadderECVarSize.empty())
        for (unsigned i=0; i<bankOption.subLadderECVarSize.size(); ++i)
            bankOption.subLadderECVarSize.at(i) = std::min(std::max(1u, bankOption.subLadderECVarSize.at(i)), 32u);
    if (!bankOption.subModuleECVarSize.empty())
        for (unsigned i=0; i<bankOption.subModuleECVarSize.size(); ++i)
            bankOption.subModuleECVarSize.at(i) = std::min(std::max(2u, bankOption.subModuleECVarSize.at(i)), 1024u);
    // Check if all variable sizes are set at the same time
    if (!bankOption.subLadderVarSize.empty() || !bankOption.subModuleVarSize.empty() ||
        !bankOption.subLadderECVarSize.empty() || !bankOption.subModuleECVarSize.empty()) {
        if (bankOption.subLadderVarSize.empty() || bankOption.subModuleVarSize.empty() ||
            bankOption.subLadderECVarSize.empty() || bankOption.subModuleECVarSize.empty()) {
            std::cout << Warning() << "Not all variable sizes are set at the same time. Ignoring all variable sizes." << std::endl;
            bankOption.subLadderVarSize.clear(); bankOption.subModuleVarSize.clear();
            bankOption.subLadderECVarSize.clear(); bankOption.subModuleECVarSize.clear();
        }
    }

    // Protection against conflict in nSubModules vs. nDCBits
    //if (bankOption.subModuleSize == 1024 && bankOption.nDCBits > 0) {
    //    std::cout << Warning() << "Requested subModuleSize: 1024, # of DC bits: " << bankOption.nDCBits << " --> " << 0 << std::endl;
    //    bankOption.nDCBits = 0;
    //}
    //if (bankOption.subModuleSize == 512 && bankOption.nDCBits > 1) {
    //    std::cout << Warning() << "Requested subModuleSize: 512, # of DC bits: " << bankOption.nDCBits << " --> " << 1 << std::endl;
    //    bankOption.nDCBits = 1;
    //}
    //if (bankOption.subModuleSize == 256 && bankOption.nDCBits > 2) {
    //    std::cout << Warning() << "Requested subModuleSize: 256, # of DC bits: " << bankOption.nDCBits << " --> " << 2 << std::endl;
    //    bankOption.nDCBits = 2;
    //}
    //if (!bankOption.subModuleVarSize.empty() && bankOption.nDCBits > 0) {
    //    std::cout << Warning() << "Requested variable subModuleSizes, # of DC bits: " << bankOption.nDCBits << " --> " << 0 << std::endl;
    //    bankOption.nDCBits = 0;
    //}

    // Check if all trigger towers are correct
    for (unsigned i=0; i<bankOption.triggerTowers.size(); ++i) {
        if (bankOption.triggerTowers.at(i) >= 48) {
            std::cout << Warning() << "Found an invalid trigger tower: " << bankOption.triggerTowers.at(i) << ". Use the default trigger towers" << std::endl;
            bankOption.triggerTowers = dv_triggerTowers;
            break;
        }
    }

    // _________________________________________________________________________
    // Calling main functions
    if (vm.count("cleanStubs")) {
        std::cout << Color("magenta") << "Parsed pattern bank options:" << EndColor() << std::endl;
        std::cout << bankOption << std::endl;
        std::cout << Color("magenta") << "Start stub cleaning..." << EndColor() << std::endl;

        StubCleaner cleaner(bankOption);
        cleaner.setNEvents(maxEvents);
        cleaner.setFilter(!nofilter);
        cleaner.setVerbosity(verbose);
        int exitcode = cleaner.run(input, output);
        if (exitcode) {
            std::cerr << "An error occurred during stub cleaning. Exiting." << std::endl;
            return exitcode;
        }

        std::cout << "Stub cleaning " << Color("lgreenb") << "DONE" << EndColor() << "." << std::endl;


    } else if (vm.count("generateBank")) {
        std::cout << Color("magenta") << "Parsed pattern bank options:" << EndColor() << std::endl;
        std::cout << bankOption << std::endl;
        std::cout << Color("magenta") << "Start pattern bank generation..." << EndColor() << std::endl;

        TString datadir = std::getenv("CMSSW_BASE");
        datadir += "/src/SLHCL1TrackTriggerSimulations/AMSimulation/data/";

        PatternGenerator generator(bankOption);
        generator.setNEvents(maxEvents);
        generator.setMinFrequency(minFrequency);
        generator.setVerbosity(verbose);
        int exitcode = generator.run(input, datadir, output);
        if (exitcode) {
            std::cerr << "An error occurred during pattern bank generation. Exiting." << std::endl;
            return exitcode;
        }

        std::cout << "Pattern bank generation " << Color("lgreenb") << "DONE" << EndColor() << "." << std::endl;

    } else if (vm.count("mergeBank")) {
        std::cout << "NOT IMPLEMENTED" << std::endl;

    } else if (vm.count("analyzeBank")) {
        std::cout << "NOT IMPLEMENTED" << std::endl;

    } else if (vm.count("patternRecognition")) {
        std::cout << Color("magenta") << "Parsed pattern bank options:" << EndColor() << std::endl;
        std::cout << bankOption << std::endl;
        std::cout << Color("magenta") << "Start pattern recognition..." << EndColor() << std::endl;

        PatternMatcher matcher(bankOption);
        matcher.setNEvents(maxEvents);
        matcher.setMinFrequency(minFrequency);
        matcher.setMaxPatterns(maxPatterns);
        matcher.setMaxRoads(maxRoads);
        matcher.setMaxHits(maxHits);
        matcher.setVerbosity(verbose);
        int exitcode = matcher.run(input, bankfile, output);
        if (exitcode) {
            std::cerr << "An error occurred during pattern recognition. Exiting." << std::endl;
            return exitcode;
        }

        std::cout << "Pattern recognition " << Color("lgreenb") << "DONE" << EndColor() << "." << std::endl;

    } else if (vm.count("trackFitting")) {
        std::cout << Color("magenta") << "Parsed track fitting options:" << EndColor() << std::endl;
        std::cout << fitOption << std::endl;
        std::cout << Color("magenta") << "Start track fitting..." << EndColor() << std::endl;

        TrackFitter fitter(fitOption);
        fitter.setNEvents(maxEvents);
        fitter.setMaxTracks(maxTracks);
        fitter.setVerbosity(verbose);
        int exitcode = fitter.run(input, output);
        if (exitcode) {
            std::cerr << "An error occurred during track fitting. Exiting." << std::endl;
            return exitcode;
        }

        std::cout << "Track fitting " << Color("lgreenb") << "DONE" << EndColor() << "." << std::endl;

    } else if (vm.count("write")) {
        std::cout << Color("magenta") << "Start writing full ntuple..." << EndColor() << std::endl;

        NTupleMaker writer;
        writer.setNEvents(maxEvents);
        writer.setTrim(!notrim);
        writer.setVerbosity(verbose);
        int exitcode = writer.run(input, roadfile, trackfile, output);
        if (exitcode) {
            std::cerr << "An error occurred during writing full ntuple. Exiting." << std::endl;
            return exitcode;
        }

        std::cout << "Writing full ntuple " << Color("lgreenb") << "DONE" << EndColor() << "." << std::endl;
    }

    return EXIT_SUCCESS;
}
