#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include "./args.hpp"
#include "./fat16_reader_the_prime.hpp"

namespace po = boost::program_options;

int main(int argc, char **argv) {
    po::options_description opt_descr{
        "Usage: " + std::string(argv[0]) + " [-h|--help] <file>\n"
        "A determined filesystem reader"};
    opt_descr.add_options()("help,h", "Show help message");
    Args parser{argc, argv, opt_descr};

    if (parser.var_map.count("help")) {
        std::cout << opt_descr << std::endl;
        return 0;
    }
    std::vector<std::string> unrecognized =
        po::collect_unrecognized(parser.parsed.options, po::include_positional);
    if (unrecognized.size() != 1) {
        std::cerr << "Exactly one file must be specified." << std::endl;
        return 1;
    }
    std::filesystem::path file = unrecognized[0];

    try {
        fat16_boot_sector boot_sector = read_boot_sector(file);
        print_boot_sector(boot_sector);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
