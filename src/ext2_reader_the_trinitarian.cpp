#include "./ext2_reader_the_trinitarian.hpp"
#include "./date_to_string.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

ext2_supablock read_supablock(std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    _ext2_supablock _supablock;
    file.seekg(1024);
    file.read(reinterpret_cast<char *>(&_supablock), sizeof(_ext2_supablock));
    ext2_supablock supablock;
    memcpy(&supablock, &_supablock,
           reinterpret_cast<size_t>(&_supablock.s_volume_name) -
               reinterpret_cast<size_t>(&_supablock));
    supablock.s_algorithm_usage_bitmap = _supablock.s_algorithm_usage_bitmap;
    supablock.s_prealloc_blocks = _supablock.s_prealloc_blocks;
    supablock.s_prealloc_dir_blocks = _supablock.s_prealloc_dir_blocks;
    supablock.s_padding1 = _supablock.s_padding1;
    strncpy(supablock.s_volume_name, _supablock.s_volume_name, 16);
    supablock.s_volume_name[16] = '\0';
    int last_nonspace;
    for (last_nonspace = 15; last_nonspace > 0; last_nonspace--) {
        if (supablock.s_volume_name[last_nonspace] != ' ') {
            break;
        }
    }
    supablock.s_volume_name[last_nonspace + 1] = '\0';
    strncpy(supablock.s_last_mounted, _supablock.s_last_mounted, 64);
    supablock.s_last_mounted[64] = '\0';
    for (last_nonspace = 63; last_nonspace > 0; last_nonspace--) {
        if (supablock.s_last_mounted[last_nonspace] != ' ') {
            break;
        }
    }
    supablock.s_last_mounted[last_nonspace + 1] = '\0';

    uint32_t block_size = supablock.s_log_block_size;
    uint64_t root_directory_offset = block_size * supablock.s_first_data_block;
    file.seekg(root_directory_offset);

    ext2_directory_entry entry;
    while (file.read(reinterpret_cast<char *>(&entry), sizeof(ext2_directory_entry))) {
        if (entry.name[0] == 0) break; 
        supablock.root_files.push_back(entry);
    }

    return supablock;
}

void print_ext2(ext2_supablock partition) {
    std::cout << "Volume name:                  " << partition.s_volume_name
              << std::endl;
    std::cout << "Last mounted dir location:    " << partition.s_last_mounted
              << std::endl;
    std::cout << "Num of inodes:                " << partition.s_inodes_count
              << std::endl;
    std::cout << "Num of blocks:                " << partition.s_blocks_count
              << std::endl;
    std::cout << "Num blocks for superuser:     " << partition.s_r_blocks_count
              << std::endl;
    std::cout << "Num of unallocated blocks:    "
              << partition.s_free_blocks_count << std::endl;
    std::cout << "Num of unallocated inodes:    "
              << partition.s_free_inodes_count << std::endl;
    std::cout << "Block num of superblock:      "
              << partition.s_first_data_block << std::endl;
    std::cout << "log2 block size:              " << partition.s_log_block_size
              << std::endl;
    std::cout << "log2 fragment size:           " << partition.s_log_frag_size
              << std::endl;
    std::cout << "Blocks per group:             "
              << partition.s_blocks_per_group << std::endl;
    std::cout << "Fragments per group:          " << partition.s_frags_per_group
              << std::endl;
    std::cout << "Inodes per group:             "
              << partition.s_inodes_per_group << std::endl;
    std::cout << "Last mount time:              "
              << date_to_string(partition.s_mtime) << std::endl;
    std::cout << "Last write time:              "
              << date_to_string(partition.s_wtime) << std::endl;
    std::cout << "Mount count since last check: " << partition.s_mnt_count
              << std::endl;
    std::cout << "Max mount count before check: " << partition.s_max_mnt_count
              << std::endl;
    std::cout << "FS state:                     "
              << (partition.s_state ? "OK" : "Errors") << std::endl;
    std::cout << "Last check time:              "
              << date_to_string(partition.s_mtime) << std::endl;
    std::cout << "Max time between checks:      " << partition.s_checkinterval
              << std::endl;
    std::cout << "Creator OS:                   "
              << (partition.s_creator_os == 0   ? "Linux"
                  : partition.s_creator_os == 3 ? "FreeBSD"
                                                : "Unknown")
              << std::endl;
    std::cout << "Minor revision:               " << partition.s_minor_rev_level
              << std::endl;
    std::cout << "Revision:                     " << partition.s_rev_level
              << std::endl;
    std::cout << "User, Group of reserved:      " << partition.s_def_resuid
              << ", " << partition.s_def_resgid << std::endl;
    std::cout << "First non-reserved inode:     " << partition.s_first_ino
              << std::endl;
    std::cout << "Inode size:                   " << partition.s_inode_size
              << std::endl;
    std::cout << "Block group of superblock:    " << partition.s_block_group_nr
              << std::endl;
    std::cout << "UUID:                         ";
    for (size_t i = 0; i < 16; i++) {
        if (i % 2 == 0 && i > 2 && i < 12) {
            std::cout << "-";
        }
        if (partition.s_uuid[i] < 16) {
            std::cout << "0";
        }
        std::cout << std::hex << static_cast<uint16_t>(partition.s_uuid[i]);
    }
    std::cout << std::endl;
    std::cout << "Root Directory Entries:\n";
    for (const auto& file : partition.root_files) {
        print_file_ext2(file, partition);
    }
}


std::string ext2_date_to_string(uint16_t date, uint16_t time_hms) {
    int year = ((date >> 9) & 0x7F) + 1980;
    int month = (date >> 5) & 0x0F;
    int day = date & 0x1F;

    int hour = (time_hms >> 11) & 0x1F;
    int minute = (time_hms >> 5) & 0x3F;
    int second = (time_hms & 0x1F) * 2;

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             year, month, day, hour, minute, second);
    return std::string(buffer);
}

void print_file_ext2(const ext2_directory_entry& file, const ext2_supablock& superblock) {
    std::cout << ((file.attributes & 0x01) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x02) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x04) ? "     +  " : "     -  ");
    std::cout << ((file.attributes & 0x08) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x0f) ? "        +  " : "        -  ");
    std::cout << ((file.attributes & 0x20) ? "      +  " : "      -  ");

    std::string cluster =
            std::to_string(file.first_cluster_low) + " (" +
            std::to_string(file.first_cluster_low * (1 << (10 + superblock.s_log_block_size))) +
            " B)";
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

    std::cout << ext2_date_to_string(file.creation_date, file.creation_time_hms)
              << ' '
              << ext2_date_to_string(file.modified_date, file.modified_time_hms)
              << "  ";

    std::cout << file.name << std::endl;
}
