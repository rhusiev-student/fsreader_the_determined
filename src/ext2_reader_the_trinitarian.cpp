#include "./ext2_reader_the_trinitarian.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
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
    return supablock;
}

void print_ext2(ext2_supablock partition) {
    std::cout << "Volume name:                  "
              << partition.s_volume_name << std::endl;
    std::cout << "Last mounted dir location:    "
              << partition.s_last_mounted << std::endl;
    std::cout << "Num of inodes:                "
              << partition.s_inodes_count << std::endl;
    std::cout << "Num of blocks:                "
              << partition.s_blocks_count << std::endl;
    std::cout << "Num blocks for superuser:     "
              << partition.s_r_blocks_count << std::endl;
    std::cout << "Num of unallocated blocks:    "
              << partition.s_free_blocks_count << std::endl;
    std::cout << "Num of unallocated inodes:    "
              << partition.s_free_inodes_count << std::endl;
    std::cout << "Block num of superblock:      "
              << partition.s_first_data_block << std::endl;
    std::cout << "log2 block size:              "
              << partition.s_log_block_size << std::endl;
    std::cout << "log2 fragment size:           "
              << partition.s_log_frag_size << std::endl;
    std::cout << "Blocks per group:             "
              << partition.s_blocks_per_group << std::endl;
    std::cout << "Fragments per group:          "
              << partition.s_frags_per_group << std::endl;
    std::cout << "Inodes per group:             "
              << partition.s_inodes_per_group << std::endl;
    std::cout << "Last mount time:              "
              << partition.s_mtime << std::endl;
}
