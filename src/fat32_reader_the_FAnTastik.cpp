#include "./fat32_reader_the_FAnTastik.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

fat32_boot_sector read_boot_sector32(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    fat32_boot_sector boot_sector{};

    file.read(reinterpret_cast<char *>(&boot_sector),
              BOOT_SECTOR_BYTES_BEFORE_CHAR_FAT32 + 11);
    boot_sector.volume_label[11] = '\0';
    file.read(reinterpret_cast<char *>(&boot_sector.filesystem_type), 8);
    boot_sector.filesystem_type[8] = '\0';

    file.seekg(510);
    uint16_t signature;
    file.read(reinterpret_cast<char*>(&signature), sizeof(uint16_t));
    boot_sector.correct_signature = (signature == 0xAA55);

    return boot_sector;
}


void print_fat32(fat32 partition) {
    std::cout << "Volume label:                 "
              << partition.boot_sector.volume_label << std::endl;
    std::cout << "Filesystem type:              "
              << partition.boot_sector.filesystem_type << std::endl;
    std::cout << "Sector size:                  "
              << partition.boot_sector.bytes_per_sector << " B" << std::endl;
    std::cout << "Sectors per cluster:          "
              << static_cast<uint16_t>(
                      partition.boot_sector.sectors_per_cluster)
              << std::endl;
    std::cout << "Num of FAT copies:            "
              << static_cast<uint16_t>(partition.boot_sector.num_fats)
              << std::endl;
    std::cout << "FAT size:                     "
              << partition.boot_sector.each_fat_size32 << " Sectors ("
              << partition.boot_sector.each_fat_size32 *
                 partition.boot_sector.bytes_per_sector
              << " B)" << std::endl;
    std::cout << "Max files in root dir:        "
              << partition.boot_sector.max_num_root_files << std::endl;
    std::cout << "Num of files in root dir:     " << partition.root_files.size()
              << std::endl;
    std::cout << "Root dir size:                "
              << partition.root_files.size() * sizeof(fat32_directory_entry)
              << " B" << std::endl;
    std::cout << "Num of reserved sectors:      "
              << partition.boot_sector.num_reserved_sectors << std::endl;
    std::cout << "There is a correct signature: "
              << (partition.boot_sector.correct_signature ? "yes" : "no")
              << std::endl;
    std::cout << "Files:" << std::endl;
    std::cout << "Read-only  " << "Hidden  " << "System  " << "Vol Label  "
              << "Long name  " << "Archive  " << "Cluster Num  "
              << "Size           " << "Creation date       "
              << "Modified date        " << "Name" << std::endl;
    for (size_t i = 0; i < partition.root_files.size(); i++) {
        print_file32(partition.root_files[i], partition.boot_sector);
    }
}

// std::vector<uint16_t> get_fat(fat16_boot_sector boot_sector,
//                               std::filesystem::path path) {}

std::vector<fat32_directory_entry> get_root_files32(const fat32_boot_sector& boot_sector,
                                                  const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    uint32_t root_start_sector = boot_sector.num_reserved_sectors +
                                 (boot_sector.num_fats * boot_sector.each_fat_size32);
    uint32_t root_start_byte = root_start_sector * boot_sector.bytes_per_sector;

    file.seekg(root_start_byte);

    std::vector<fat32_directory_entry> root_files;
    while (true) {
        fat32_directory_entry entry{};
        file.read(reinterpret_cast<char*>(&entry), sizeof(fat32_directory_entry));
        if (entry.name[0] == 0x00) break; //end, no more valid entries exist
        if (entry.name[0] == 0xE5) continue;
        root_files.push_back(entry);
    }

    return root_files;
}

std::string fatdate_to_string32(uint16_t date, uint16_t time) {
    std::string day = std::to_string(date & 0x1F);
    std::string month = std::to_string((date >> 5) & 0x0F);
    std::string year = std::to_string(1980 + ((date >> 9) & 0x7F));
    std::string hours = std::to_string((time >> 11) & 0x1F);
    std::string minutes = std::to_string((time >> 5) & 0x3F);
    std::string seconds = std::to_string(2 * (time & 0x1F));
    return year + '-' + (month.size() == 1 ? "0" + month : month) + '-' +
           (day.size() == 1 ? "0" + day : day) + ' ' +
           (hours.size() == 1 ? "0" + hours : hours) + ':' +
           (minutes.size() == 1 ? "0" + minutes : minutes) + ':' +
           (seconds.size() == 1 ? "0" + seconds : seconds);
}

void print_file32(const fat32_directory_entry& file, const fat32_boot_sector& boot_sector) {
    std::cout << ((file.attributes & 0x01) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x02) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x04) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x08) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x0f) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x20) ? "      +  " : "      -  ");

    std::string cluster =
            std::to_string(file.first_cluster_low) + " (" +
            std::to_string(file.first_cluster_low * boot_sector.sectors_per_cluster) +
            ")  ";
    int to_add_spaces = 12 - cluster.size();
    std::cout << std::string((to_add_spaces >= 0 ? to_add_spaces : 0), ' ')
              << cluster;

    if (!(file.attributes & 0x10)) {
        std::string file_size = std::to_string(file.file_size);
        int to_add_spaces = 12 - file_size.size();
        std::cout << std::string((to_add_spaces >= 0 ? to_add_spaces : 0), ' ')
                  << file_size << " B  ";
    } else {
        std::cout << "           DIR  ";
    }

    std::cout << fatdate_to_string32(file.creation_date, file.creation_time_hms)
              << ' '
              << fatdate_to_string32(file.modified_date, file.modified_time_hms)
              << "  ";

    std::cout << file.name << std::endl;
}
