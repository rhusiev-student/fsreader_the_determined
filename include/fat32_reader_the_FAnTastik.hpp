#ifndef INCLUDE_FAT32_READER_THE_FANTASTIC_HPP_
#define INCLUDE_FAT32_READER_THE_FANTASTIC_HPP_

#include <cstdint>
#include <filesystem>
#include <vector>

#pragma pack(push, 1)

struct fat32_boot_sector {
    uint8_t bootjmp[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t max_num_root_files;
    uint16_t num_sectors1;
    uint8_t media_type;
    uint16_t each_fat_size16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t num_sectors_before_partition_start;
    uint32_t num_sectors2;
    uint32_t each_fat_size32;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t bios_drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_serial_number;
    char volume_label[11 + 1];   // +1 for '\0'
    char filesystem_type[8 + 1]; // +1 for '\0'
    bool correct_signature;
};

struct _fat32_directory_entry {
    char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_in_tensecs;
    uint16_t creation_time_hms;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t modified_time_hms;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
};

struct fat32_directory_entry {
    char name[12];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_in_tensecs;
    uint16_t creation_time_hms;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t modified_time_hms;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t file_size;

    fat32_directory_entry(const _fat32_directory_entry& entry);
};

#pragma pack(pop)

struct fat32 {
    fat32_boot_sector boot_sector;
    std::vector<uint16_t> fat;
    std::vector<fat32_directory_entry> root_files;
};

#define BOOT_SECTOR_BYTES_BEFORE_CHAR_FAT32 71
#define DIRECTORY_ENTRY_BYTES_BEFORE_CHAR_FAT32 21
#define MAX_FILE_LEN_FAT32 13

fat32_boot_sector read_boot_sector32(const std::filesystem::path& path);

void print_fat32(fat32 partition);

std::vector<uint16_t> get_fat32(fat32_boot_sector boot_sector,
                              std::filesystem::path path);

std::vector<fat32_directory_entry> get_root_files32(const fat32_boot_sector& boot_sector,
                                                  const std::filesystem::path& path);

void print_file32(const fat32_directory_entry& file, const fat32_boot_sector& boot_sector);

#endif // INCLUDE_FAT32_READER_THE_FANTASTIC_HPP_
