#include "./fat16_reader_the_prime.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

fat16_boot_sector read_boot_sector(std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    fat16_boot_sector boot_sector;
    file.read(reinterpret_cast<char *>(&boot_sector), BYTES_BEFORE_CHARS + 11);
    boot_sector.volume_label[11] = '\0';
    file.read(reinterpret_cast<char *>(&boot_sector.filesystem_type), 8);
    boot_sector.filesystem_type[8] = '\0';
    file.seekg(510);
    uint16_t signature;
    file.read(reinterpret_cast<char *>(&signature), sizeof(uint16_t));
    if (signature != 0xAA55) {
        throw std::runtime_error("Invalid boot sector signature");
    }
    return boot_sector;
}

void print_boot_sector(fat16_boot_sector boot_sector) {
    std::cout << "Filesystem type: " << boot_sector.filesystem_type
              << std::endl;
    std::cout << "Volume label: " << boot_sector.volume_label << std::endl;
}
