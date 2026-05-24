#include "command_line_parser.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
namespace command {

std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("tick-period,t", po::value<int>(), "set tick period (ms)")
        ("config-file,c", po::value<std::string>(), "set config file path")
        ("www-root,w", po::value<std::string>(), "set static files root")
        ("randomize-spawn-points", "spawn dogs at random positions")
        ("state-file,f", po::value<std::string>(), "path to state file")
        ("save-state-period,p", po::value<int>(), "auto-save period (ms)");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    Args args;

    if (vm.count("help")) {
        args.help = true;
        std::cout << desc << std::endl;
        return args; // help обрабатывается в main
    }

    if (!vm.count("config-file")) {
        std::cerr << "Missing required parameter --config-file" << std::endl;
        std::cout << desc << std::endl;
        return std::nullopt;
    }
    if (!vm.count("www-root")) {
        std::cerr << "Missing required parameter --www-root" << std::endl;
        std::cout << desc << std::endl;
        return std::nullopt;
    }

    args.config_file = vm["config-file"].as<std::string>();
    args.www_root = vm["www-root"].as<std::string>();

    if (vm.count("tick-period")) {
        int period = vm["tick-period"].as<int>();
        if (period <= 0) {
            std::cerr << "tick-period must be positive" << std::endl;
            return std::nullopt;
        }
        args.tick_period = period;
    }

    if (vm.count("state-file")) {
        args.state_file = std::filesystem::path(vm["state-file"].as<std::string>());
    }

    if (vm.count("save-state-period")) {
        int period_ms = vm["save-state-period"].as<int>();
        if (period_ms <= 0) {
            std::cerr << "save-state-period must be positive" << std::endl;
            return std::nullopt;
        }
        args.save_state_period = std::chrono::milliseconds(period_ms);
    }

    args.random_spawn = vm.count("randomize-spawn-points") > 0;

    return args;
}

}
