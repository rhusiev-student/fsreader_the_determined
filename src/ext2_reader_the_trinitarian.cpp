#include "./ext2_reader_the_trinitarian.hpp"
#include "./date_to_string.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

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
}
