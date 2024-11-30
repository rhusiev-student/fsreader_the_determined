#ifndef INCLUDE_FAT16_READER_THE_PRIME_HPP_
#define INCLUDE_FAT16_READER_THE_PRIME_HPP_

#include <cstdint>
#include <filesystem>

#pragma pack(push, 1)
struct fat16_boot_sector { // alignas(16)
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
    uint8_t empty;
    uint8_t extended_boot_signature;
    uint32_t volume_serial_number;
    char volume_label[11 + 1]; // +1 for '\0'
    char filesystem_type[8 + 1]; // +1 for '\0'
};
#pragma pack(pop)

#define BYTES_BEFORE_CHARS 42

fat16_boot_sector read_boot_sector(std::filesystem::path path);

void print_boot_sector(fat16_boot_sector boot_sector);

#endif // INCLUDE_FAT16_READER_THE_PRIME_HPP_
