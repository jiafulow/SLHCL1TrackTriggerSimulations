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
        ("patternRecognition,R", "Run associative memory pattern recognition")
        ("trackFitting,T"      , "Perform track fitting")
        ("write,W"             , "Write full ntuple")
        ("verbosity,v"         , po::value<int>(&verbose)->default_value(1), "Verbosity level (-1 = very quiet; 0 = quiet, 1 = verbose, 2+ = debug)")
        ("no-color"            , "Turn off colored text")
        ("timing"              , "Show timing information")
        ;

    // Declare a group of options that will be allowed both on command line
    // and in config file
    std::string input, output, bankfile, roadfile, trackfile;
    long long maxEvents;
    int minFrequency, maxPatterns, maxMisses, maxStubs, maxRoads, maxCombs, maxTracks;
    bool notrim;
    PatternBankOption bankOption;
    TrackFitterOption fitOption;

    po::options_description config("Configuration");
    config.add_options()
        ("input,i"      , po::value<std::string>(&input)->required(), "Specify input files")
        ("output,o"     , po::value<std::string>(&output)->required(), "Specify output file")
        ("maxEvents,n"  , po::value<long long>(&maxEvents)->default_value(-1), "Specfiy max number of events")

        // Only for bank generation
        ("minFrequency" , po::value<int>(&minFrequency)->default_value(1), "Specify min frequency of a pattern to be valid")

        // Only for pattern matching
        ("bank,B"       , po::value<std::string>(&bankfile), "Specify pattern bank file")
        ("maxPatterns"  , po::value<int>(&maxPatterns)->default_value(-1), "Specfiy max number of patterns")
        ("maxMisses"    , po::value<int>(&maxMisses)->default_value(0), "Specify max number of allowed misses")
        ("maxStubs"     , po::value<int>(&maxStubs)->default_value(-1), "Specfiy max number of stubs per superstrip")
        ("maxRoads"     , po::value<int>(&maxRoads)->default_value(-1), "Specfiy max number of roads per event")

        // Only for track fitting
        ("maxCombs"     , po::value<int>(&maxCombs)->default_value(-1), "Specfiy max number of combinations per road")
        ("maxTracks"    , po::value<int>(&maxTracks)->default_value(-1), "Specfiy max number of tracks per event")

        // Only for writing full ntuple
        ("no-trim"      , po::bool_switch(&notrim)->default_value(false), "Do not trim TTree branches")
        ("roads"        , po::value<std::string>(&roadfile), "Specify file containing the roads")
        ("tracks"       , po::value<std::string>(&trackfile), "Specify file containing the tracks")

        // Specifically for a pattern bank
        ("bank_superstrip,s"        , po::value<std::string>(&bankOption.superstrip)->default_value("ss256_nz2"), "Specify the superstrip definition (default: ss256_nz2)")
        ("bank_minPt"               , po::value<float>(&bankOption.minPt)->default_value(     2.0), "Specify min pt")
        ("bank_maxPt"               , po::value<float>(&bankOption.maxPt)->default_value(999999.0), "Specify max pt")
        ("bank_minEta"              , po::value<float>(&bankOption.minEta)->default_value(-2.5), "Specify min eta (signed)")
        ("bank_maxEta"              , po::value<float>(&bankOption.maxEta)->default_value( 2.5), "Specify max eta (signed)")
        ("bank_minPhi"              , po::value<float>(&bankOption.minPhi)->default_value(-M_PI), "Specify min phi (from -pi to pi)")
        ("bank_maxPhi"              , po::value<float>(&bankOption.maxPhi)->default_value( M_PI), "Specify max phi (from -pi to pi)")
        ("bank_nLayers"             , po::value<unsigned>(&bankOption.nLayers)->default_value(6), "Specify # of layers")
        ("bank_nFakers"             , po::value<unsigned>(&bankOption.nFakers)->default_value(0), "Specify # of fake superstrips")
        ("bank_nDCBits"             , po::value<unsigned>(&bankOption.nDCBits)->default_value(0), "Specify # of DC bits")
        ("bank_tower"               , po::value<unsigned>(&bankOption.tower)->default_value(27), "Specify the trigger tower")
        ("bank_mode"                , po::value<unsigned>(&bankOption.mode)->default_value(0), "Select superstrip config")

        // Specifically for a track fitter
        ("fit_maxChi2Red"           , po::value<float>(&fitOption.maxChi2Red)->default_value(999.), "Specify maximum reduced chi-squared")
        ("fit_minNdof"              , po::value<int>(&fitOption.minNdof)->default_value(0), "Specify minimum degree of freedom")
        ("fit_mode"                 , po::value<unsigned>(&fitOption.mode)->default_value(0), "Select track fitter config -- 0: Linearized; 1: Das")
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

    // At least one option needs to be called
    if (!(vm.count("cleanStubs")         ||
          vm.count("generateBank")       ||
          vm.count("mergeBank")          ||
          vm.count("patternRecognition") ||
          vm.count("trackFitting")       ||
          vm.count("write")              ) ) {
        std::cout << "Must select one of '-C', '-G', '-M', '-R', '-T' or 'W'" << std::endl;
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
    bankOption.nLayers          = std::min(std::max(3u, bankOption.nLayers), 8u);
    bankOption.nFakers          = std::min(std::max(0u, bankOption.nFakers), 3u);
    bankOption.nDCBits          = std::min(std::max(0u, bankOption.nDCBits), 4u);


    // _________________________________________________________________________
    // Calling the producers

    TString datadir = std::getenv("CMSSW_BASE");
    datadir += "/src/SLHCL1TrackTriggerSimulations/AMSimulation/data/";

    if (vm.count("cleanStubs")) {
        std::cout << Color("magenta") << "Parsed pattern bank options:" << EndColor() << std::endl;
        std::cout << bankOption << std::endl;
        std::cout << Color("magenta") << "Start stub cleaning..." << EndColor() << std::endl;

        StubCleaner cleaner(bankOption);
        cleaner.setNEvents(maxEvents);
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

    } else if (vm.count("patternRecognition")) {
        std::cout << Color("magenta") << "Parsed pattern bank options:" << EndColor() << std::endl;
        std::cout << bankOption << std::endl;
        std::cout << Color("magenta") << "Start pattern recognition..." << EndColor() << std::endl;

        PatternMatcher matcher(bankOption);
        matcher.setNEvents(maxEvents);
        matcher.setMinFrequency(minFrequency);
        matcher.setMaxPatterns(maxPatterns);
        matcher.setMaxMisses(maxMisses);
        matcher.setMaxStubs(maxStubs);
        matcher.setMaxRoads(maxRoads);
        matcher.setVerbosity(verbose);
        int exitcode = matcher.run(input, bankfile, datadir, output);
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
        fitter.setMaxCombinations(maxCombs);
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
