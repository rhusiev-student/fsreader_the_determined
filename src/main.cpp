#include "./args.hpp"
#include "./fat16_reader_the_prime.hpp"
#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace po = boost::program_options;

int main(int argc, char **argv) {
    po::options_description opt_descr{"Usage: " + std::string(argv[0]) +
                                      " [-h|--help] <file>\n"
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
    std::filesystem::path filename = unrecognized[0];

    fat16 partition;
    partition.boot_sector = read_boot_sector(filename);
    partition.root_files =
        get_root_files(partition.boot_sector, filename);
    print_fat(partition);

    return 0;
}
