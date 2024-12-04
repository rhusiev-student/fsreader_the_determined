#ifndef INCLUDE_EXFAT_READER_THE_BREATHTAKING_HPP_
#define INCLUDE_EXFAT_READER_THE_BREATHTAKING_HPP_

#include <cstdint>
#include <filesystem>
#include <vector>

#pragma pack(push, 1)

struct exfat_boot_sector {
    uint8_t bootjmp[3];                    // Jump instruction for BIOS to start execution
    uint16_t bytes_per_sector;             // Bytes per sector
    uint8_t sectors_per_cluster;           // Number of sectors per cluster
    uint16_t num_reserved_sectors;         // Number of reserved sectors
    uint8_t num_fats;                      // Number of FAT tables
    uint16_t fat_version;                  // FAT version
    uint32_t root_cluster;                 // Cluster number of the root directory
    uint16_t fs_info_sector;               // Sector number of FS info sector
    uint16_t backup_boot_sector;           // Sector number of FS info sector
    uint8_t reserved1;                     // Reserved byte
    char volume_label[12];                 // Volume label (11 characters + /0)
    char filesystem_type[9];               // Filesystem type
    bool correct_signature;                // To check if the signature is valid
};

struct exfat_directory_entry {
    char name[12];                         // Filename (11 characters + /0)
    uint8_t attributes;                    // File attributes
    uint8_t reserved;                      // Reserved byte
    uint8_t creation_time_in_tensecs;      // Creation time in 10ms units
    uint16_t creation_time_hms;            // Creation time in hours, minutes, seconds
    uint16_t creation_date;                // Creation date
    uint16_t access_date;                  // Last access date
    uint16_t first_cluster_high;           // High 16 bits of the first cluster number
    uint16_t modified_time_hms;            // Modified time in hours, minutes, seconds
    uint16_t modified_date;                // Modified date
    uint16_t first_cluster_low;            // Low 16 bits of the first cluster number
    uint32_t file_size;                    // File size in bytes
};

#pragma pack(pop)

struct exfat {
    exfat_boot_sector boot_sector_ex;
    std::vector<uint16_t> exfat;
    std::vector<exfat_directory_entry> root_files;
};

#define BOOT_SECTOR_BYTES_BEFORE_CHAR_EXFAT 19
#define DIRECTORY_ENTRY_BYTES_BEFORE_CHAR_EXFAT 21
#define MAX_FILE_LEN_EXFAT 13

exfat_boot_sector read_boot_sector_exfat(const std::filesystem::path& path);

void print_exfat(exfat partition);

std::vector<uint16_t> get_exfat(exfat_boot_sector  boot_sector_ex,
                              std::filesystem::path path);

std::vector<exfat_directory_entry> get_root_files_exfat(const exfat_boot_sector&  boot_sector_ex,
                                                  const std::filesystem::path& path);

void print_file_exfat(const exfat_directory_entry& file, const exfat_boot_sector&  boot_sector_ex);

#endif // INCLUDE_EXFAT_READER_THE_BREATHTAKING_HPP_