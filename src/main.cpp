#include "./args.hpp"
#include "./fat16_reader_the_prime.hpp"
#include "./fat32_reader_the_FAnTastik.hpp"
#include "./ext2_reader_the_trinitarian.hpp"
#include "./fat12_reader_the_marvelous.hpp"
#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

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

    std::ifstream file(filename, std::ios::binary);

    auto filesize = std::filesystem::file_size(filename);
    if (filesize < 1024) {
        std::cout << "Not fat12, fat16, fat32 or ext2" << std::endl;
        return 0;
    }
    if (filesize >= 2048) {
        ext2_supablock supablock = read_supablock(filename);
        if (supablock.s_magic == 0xEF53) {
            std::cout << "Found ext2" << std::endl;
            print_ext2(supablock);
            return 0;
        }
    }

    fat32_boot_sector boot_sector{};
    file.read(reinterpret_cast<char*>(&boot_sector), sizeof(fat32_boot_sector));

    uint32_t root_dir_sectors = ((boot_sector.max_num_root_files * 32) +
                                 (boot_sector.bytes_per_sector - 1)) /
                                boot_sector.bytes_per_sector;
    //divides the total size by the number of bytes per sector to get the number of sectors used for the root directory
    uint32_t total_sectors = (boot_sector.num_sectors1 != 0)
                             ? boot_sector.num_sectors1
                             : boot_sector.num_sectors2;
    //calculates the total number of sectors in the filesystem. If num_sectors1 is non-zero, it's used;
    // otherwise, num_sectors2 is used as the total number of sectors
    uint32_t fat_size = (boot_sector.each_fat_size16 != 0)
                        ? boot_sector.each_fat_size16
                        : boot_sector.each_fat_size32;
    //calculates the size of each FAT table
    uint32_t data_sectors = total_sectors -
                            (boot_sector.num_reserved_sectors +
                             (boot_sector.num_fats * fat_size) +
                             root_dir_sectors);
    //calculates the number of sectors dedicated to data. It subtracts the reserved sectors, the FAT tables,
    // and the root directory sectors from the total sectors to get the remaining data sectors.
    uint64_t cluster_count = data_sectors / boot_sector.sectors_per_cluster;
    //calculates the number of clusters by dividing the number of data sectors by the sectors per cluster

   if (cluster_count  <= 4084) {
        // FAT12
        fat12 partition;
        partition.boot_sector = read_boot_sector12(filename);
        partition.root_files = get_root_files12(partition.boot_sector, filename);
        print_fat12(partition);
    }
    else if  (4084 <= cluster_count && cluster_count <= 65524) {
        // FAT16
        std::cout << "Found FAT16" << std::endl;
        fat16 partition;
        partition.boot_sector = read_boot_sector(filename);
        partition.root_files = get_root_files(partition.boot_sector, filename);
        print_fat(partition);
    } else if (65525 <= cluster_count && cluster_count <= 4294967295) {
        // FAT32
        std::cout << "Found FAT32" << std::endl;
        fat32 partition;
        partition.boot_sector = read_boot_sector32(filename);
        partition.root_files = get_root_files32(partition.boot_sector, filename);
        print_fat32(partition);
    } else {
        std::cout << "This is not supported" << std::endl;
    }

    return 0;
}
