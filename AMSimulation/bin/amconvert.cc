#include "SLHCL1TrackTriggerSimulations/AMSimulation/interface/Converter.h"

#include "boost/program_options.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <exception>
#include <memory>
#include <string>


// _____________________________________________________________________________
int main(int argc, char **argv) {

    // Declare a group of options that will be allowed only on command line
    namespace po = boost::program_options;
    int verbose;
    po::options_description generic("Generic options");
    generic.add_options()
        ("version"             , "Print version")
        ("help,h"              , "Produce help message")
        ("stubs,S"             , "Convert from a root file containing stubs info to a text file")
        ("bank,B"              , "Convert from a root file containing pattern bank to a text file")
        ("verbosity,v"         , po::value<int>(&verbose)->default_value(1), "Verbosity level (-1 = very quiet; 0 = quiet, 1 = verbose, 2+ = debug)")
        ("no-color"            , "Turn off colored text")
        ("timing"              , "Show timing information")
        ;

    // Declare a group of options that will be allowed both on command line
    // and in config file
    std::string input, output, format;
    long long maxEvents, skipEvents;
    po::options_description config("Configuration");
    config.add_options()
        ("input,i"      , po::value<std::string>(&input)->required(), "Specify input files")
        ("output,o"     , po::value<std::string>(&output)->required(), "Specify output file")
        ("format"       , po::value<std::string>(&format)->default_value("x"), "Specify output format")
        ("maxEvents,n"  , po::value<long long>(&maxEvents)->default_value(-1), "Specfiy max number of events")
        ("skipEvents,m" , po::value<long long>(&skipEvents)->default_value(-1), "Specfiy number of first events to skip")
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
    if (!(vm.count("stubs")         ||
          vm.count("bank")          ) ) {
        std::cout << "Must select one of '-S' or '-B'" << std::endl;
        std::cout << visible << std::endl;
        return 1;
    }

    if (vm.count("no-color")) {
        slhcl1tt::NoColor();
    }

    if (vm.count("timing")) {
        slhcl1tt::ShowTiming();
    }

    // _________________________________________________________________________
    // Calling main functions
    if (vm.count("stubs")) {
        std::cout << "NOT IMPLEMENTED" << std::endl;

    } else if (vm.count("bank")) {
        std::cout << Color("magenta") << "Start converting bank..." << EndColor() << std::endl;

        Converter converter;
        converter.setNEvents(maxEvents);
        converter.setSkipEvents(skipEvents);
        converter.setVerbosity(verbose);

        converter.bankToTxt(input, output, format);

        std::cout << "Bank conversion " << Color("lgreenb") << "DONE" << EndColor() << "." << std::endl;
    }

    return EXIT_SUCCESS;
}
