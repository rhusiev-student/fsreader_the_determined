#ifndef INCLUDE_FAT12_READER_THE_TRINITARIAN_HPP_
#define INCLUDE_FAT12_READER_THE_TRINITARIAN_HPP_

#include <cstdint>
#include <filesystem>
#include <vector>

#pragma pack(push, 1)
struct fat12_boot_sector { // alignas(16)
    uint8_t bootjmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t max_num_root_files;
    uint16_t num_sectors1;
    uint8_t media_type;
    uint16_t each_fat_size;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t num_sectors_before_partition_start;
    uint32_t num_sectors2;
    uint8_t bios_drive_number;
    uint8_t extended_boot_signature;
    uint32_t volume_serial_number;
    // here ends BOOT_SECTOR_BYTES_BEFORE_CHAR
    char volume_label[11 + 1];   // +1 for '\0'
    char filesystem_type[8 + 1]; // +1 for '\0'
    bool correct_signature;
};

struct _fat12_directory_entry {
    char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_in_tensecs;
    uint16_t creation_time_hms;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t first_cluster2;
    uint16_t modified_time_hms;
    uint16_t modified_date;
    uint16_t first_cluster1;
    uint32_t file_size;
};

struct fat12_directory_entry {
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_in_tensecs;
    uint16_t creation_time_hms;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t first_cluster2;
    uint16_t modified_time_hms;
    uint16_t modified_date;
    uint16_t first_cluster1;
    uint32_t file_size;
    char name[11 + 3]; // +2 for '\0', +1 for '.'
};

#pragma pack(pop)

struct fat12 {
    fat12_boot_sector boot_sector;
    std::vector<uint16_t> fat;
    std::vector<fat12_directory_entry> root_files;
};

#define BOOT_SECTOR_BYTES_BEFORE_CHAR 42 //to change
#define DIRECTORY_ENTRY_BYTES_BEFORE_CHAR 21

fat12_boot_sector read_boot_sector12(std::filesystem::path path);

void print_fat12(fat12 partition);

std::vector<uint16_t> get_fat12(fat12_boot_sector boot_sector,
                              std::filesystem::path path);

std::vector<fat12_directory_entry> get_root_files12(fat12_boot_sector boot_sector,
                                                  std::filesystem::path path);

void print_file12(fat12_directory_entry file, fat12_boot_sector boot_sector);

#endif // INCLUDE_FAT12_READER_THE_TRINITARIAN_HPP_
