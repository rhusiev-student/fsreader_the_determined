#include "./exfat_reader_the_breathtaking.hpp"
#include "./date_to_string.hpp"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

exfat_boot_sector read_boot_sector_exfat(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    exfat_boot_sector  boot_sector_ex{};

    file.read(reinterpret_cast<char *>(& boot_sector_ex),
              BOOT_SECTOR_BYTES_BEFORE_CHAR_EXFAT + 11);
     boot_sector_ex.volume_label[11] = '\0';
    file.read(reinterpret_cast<char *>(& boot_sector_ex.filesystem_type), 8);
     boot_sector_ex.filesystem_type[8] = '\0';

    file.seekg(510);
    uint16_t signature;
    file.read(reinterpret_cast<char*>(&signature), sizeof(uint16_t));
     boot_sector_ex.correct_signature = (signature == 0xAA55);

    return  boot_sector_ex;
}


void print_fat32(exfat partition) {
   std::cout << "Volume label:                 "
              << partition. boot_sector_ex.volume_label << std::endl;
    std::cout << "Filesystem type:              "
              << partition. boot_sector_ex.filesystem_type << std::endl;
    std::cout << "Sector size:                  "
              << partition. boot_sector_ex.bytes_per_sector << " B" << std::endl;
    std::cout << "Sectors per cluster:          "
              << static_cast<uint16_t>(
                      partition. boot_sector_ex.sectors_per_cluster)
              << std::endl;
    std::cout << "Num of FAT copies:            "
              << static_cast<uint16_t>(partition. boot_sector_ex.num_fats)
              << std::endl;
    std::cout << "FAT version:                  "
              << partition. boot_sector_ex.fat_version << std::endl;
    std::cout << "Root cluster:                 "
              << partition. boot_sector_ex.root_cluster << std::endl;
    std::cout << "FS info sector:               "
              << partition. boot_sector_ex.fs_info_sector << std::endl;
    std::cout << "Backup boot sector:           "
              << partition. boot_sector_ex.backup_boot_sector << std::endl;
    std::cout << "Num of reserved sectors:      "
              << partition. boot_sector_ex.num_reserved_sectors << std::endl;
    std::cout << "There is a correct signature: "
              << (partition. boot_sector_ex.correct_signature ? "yes" : "no")
              << std::endl;
    std::cout << "Files:" << std::endl;
    std::cout << "Read-only  " << "Hidden  " << "System  " << "Vol Label  "
              << "Long name  " << "Archive  " << "Cluster Num  "
              << "Size           " << "Creation date       "
              << "Modified date        " << "Name" << std::endl;
    for (size_t i = 0; i < partition.root_files.size(); i++) {
        print_file_exfat(partition.root_files[i], partition. boot_sector_ex);
    }
}

std::vector<exfat_directory_entry> get_root_files32(const exfat_boot_sector&  boot_sector_ex,
                                                  const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the file.");
    }

    // start of the root directory
    uint32_t fat_size_in_sectors = (boot_sector_ex.num_reserved_sectors +  boot_sector_ex.num_fats *  boot_sector_ex.bytes_per_sector) /  boot_sector_ex.bytes_per_sector;

    uint32_t root_start_sector =  boot_sector_ex.num_reserved_sectors + ( boot_sector_ex.num_fats * fat_size_in_sectors);
    
    uint32_t root_start_byte = root_start_sector *  boot_sector_ex.bytes_per_sector;

    file.seekg(root_start_byte);

    std::vector<exfat_directory_entry> root_files;
    while (true) {
        exfat_directory_entry entry{};
        file.read(reinterpret_cast<char*>(&entry), sizeof(exfat_directory_entry));
        if (entry.name[0] == 0x00) break; // end, no more valid entries
        if (entry.name[0] == static_cast<char>(0xE5)) continue;
        root_files.push_back(entry);
    }

    return root_files;
}


void print_file_exfat(const exfat_directory_entry& file, const exfat_boot_sector&  boot_sector_ex) {
    std::cout << ((file.attributes & 0x01) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x02) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x04) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x08) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x0f) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x20) ? "      +  " : "      -  ");

    std::string cluster =
            std::to_string(file.first_cluster_low) + " (" +
            std::to_string(file.first_cluster_low *  boot_sector_ex.sectors_per_cluster) +
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

    std::cout << date_to_string(file.creation_date, file.creation_time_hms)
              << ' '
              << date_to_string(file.modified_date, file.modified_time_hms)
              << "  ";

    std::cout << file.name << std::endl;
}
