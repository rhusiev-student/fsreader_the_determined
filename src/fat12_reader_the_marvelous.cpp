#include "./fat12_reader_the_marvelous.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

fat12_boot_sector read_boot_sector12(std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    fat12_boot_sector boot_sector;
    file.read(reinterpret_cast<char *>(&boot_sector),
              BOOT_SECTOR_BYTES_BEFORE_CHAR + 11);
    boot_sector.volume_label[11] = '\0';
    file.read(reinterpret_cast<char *>(&boot_sector.filesystem_type), 8);
    boot_sector.filesystem_type[8] = '\0';
    file.seekg(510);
    uint16_t signature;
    file.read(reinterpret_cast<char *>(&signature), sizeof(uint16_t));
    boot_sector.correct_signature = true;
    if (signature != 0xAA55) {
        boot_sector.correct_signature = false;
    }
    return boot_sector;
}

void print_fat12(fat12 partition) {
    std::cout << "Volume label:                 "
              << partition.boot_sector.volume_label << std::endl;
    std::cout << "Filesystem type:             "
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
              << partition.boot_sector.each_fat_size << " Sectors ("
              << partition.boot_sector.each_fat_size *
                 partition.boot_sector.bytes_per_sector
              << " B)" << std::endl;
    std::cout << "Max files in root dir:        "
              << partition.boot_sector.max_num_root_files << std::endl;
    std::cout << "Num of files in root dir:     " << partition.root_files.size()
              << std::endl;
    std::cout << "Root dir size:                "
              << partition.root_files.size() * sizeof(fat12_directory_entry)
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
        print_file12(partition.root_files[i], partition.boot_sector);
    }
}

// std::vector<uint16_t> get_fat(fat16_boot_sector boot_sector,
//                               std::filesystem::path path) {}

std::vector<fat12_directory_entry> get_root_files12(fat12_boot_sector boot_sector,
                                                    std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);

    file.seekg(boot_sector.num_reserved_sectors * boot_sector.bytes_per_sector +
               boot_sector.each_fat_size * boot_sector.bytes_per_sector *
               boot_sector.num_fats);

    std::vector<_fat12_directory_entry> root_files_raw(boot_sector.max_num_root_files);
    file.read(reinterpret_cast<char *>(root_files_raw.data()),
              boot_sector.max_num_root_files * sizeof(_fat12_directory_entry));

    std::vector<fat12_directory_entry> root_files;
    root_files.reserve(boot_sector.max_num_root_files);

    for (int i = 0; i < boot_sector.max_num_root_files; i++) {
        const _fat12_directory_entry& raw_entry = root_files_raw[i];

        if (raw_entry.name[0] == static_cast<char>(0x00) || raw_entry.name[0] == static_cast<char>(0xE5)) {
            continue;
        }

        fat12_directory_entry dir_entry;
        memcpy(&dir_entry, &raw_entry.attributes, DIRECTORY_ENTRY_BYTES_BEFORE_CHAR);

        dir_entry.name[11] = '\0';
        dir_entry.name[12] = '\0';
        strncpy(dir_entry.name, raw_entry.name, 11);

        int last_nonspace;
        for (last_nonspace = 7; last_nonspace > 0; last_nonspace--) {
            if (dir_entry.name[last_nonspace] != ' ') {
                break;
            }
        }
        char extension[4] = {'\0', '\0', '\0', '\0'};
        for (int j = 2; j >= 0; j--) {
            extension[j] = dir_entry.name[8 + j];
            if (dir_entry.name[8 + j] == ' ') {
                extension[j] = '\0';
            }
        }

        strncpy(&dir_entry.name[last_nonspace + 2], extension, 4);
        dir_entry.name[last_nonspace + 1] = '.';
        if (extension[0] == '\0') {
            dir_entry.name[last_nonspace + 1] = '\0';
        }
        dir_entry.name[last_nonspace + 5] = '\0';

        root_files.push_back(dir_entry);
    }

    return root_files;
}


#define MAX_FILE_LEN 13

std::string fatdate_to_string12(uint16_t date, uint16_t time) {
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

void print_file12(fat12_directory_entry file, fat12_boot_sector boot_sector) {
    std::cout << ((file.attributes & 0x01) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x02) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x04) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x08) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x0f) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x20) ? "      +  " : "      -  ");

    std::string cluster =
            std::to_string(file.first_cluster1) + " (" +
            std::to_string(file.first_cluster1 * boot_sector.sectors_per_cluster) +
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

    std::cout << fatdate_to_string12(file.creation_date, file.creation_time_hms)
              << ' '
              << fatdate_to_string12(file.modified_date, file.modified_time_hms)
              << "  ";

    std::cout << file.name << std::endl;
}
