#include "./ext2_reader_the_trinitarian.hpp"
#include "./date_to_string.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

ext2_supablock::ext2_supablock(const _ext2_supablock &_supablock) {
    memcpy(this, &_supablock,
           reinterpret_cast<size_t>(&_supablock.s_volume_name) -
               reinterpret_cast<size_t>(&_supablock));
    s_algorithm_usage_bitmap = _supablock.s_algorithm_usage_bitmap;
    s_prealloc_blocks = _supablock.s_prealloc_blocks;
    s_prealloc_dir_blocks = _supablock.s_prealloc_dir_blocks;
    s_padding1 = _supablock.s_padding1;
    strncpy(s_volume_name, _supablock.s_volume_name, 16);
    s_volume_name[16] = '\0';
    int last_nonspace;
    for (last_nonspace = 15; last_nonspace > 0; last_nonspace--) {
        if (s_volume_name[last_nonspace] != ' ') {
            break;
        }
    }
    s_volume_name[last_nonspace + 1] = '\0';
    strncpy(s_last_mounted, _supablock.s_last_mounted, 64);
    s_last_mounted[64] = '\0';
    int last_nonspace2;
    for (last_nonspace2 = 63; last_nonspace2 > 0; last_nonspace2--) {
        if (s_last_mounted[last_nonspace2] != ' ') {
            break;
        }
    }
    s_last_mounted[last_nonspace2 + 1] = '\0';
}

ext2_supablock read_supablock(std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    _ext2_supablock _supablock;
    file.seekg(1024);
    file.read(reinterpret_cast<char *>(&_supablock), sizeof(_ext2_supablock));
    ext2_supablock supablock(_supablock);

    return supablock;
}

void add_files(ext2_supablock &supablock, std::filesystem::path path) {
    std::ifstream file(path, std::ios::binary);
    size_t block_size = (1 << supablock.s_log_block_size) * 1024;

    size_t root_block_group_location;
    if (block_size >= 2048) {
        root_block_group_location = block_size;
    } else if (block_size <= 1024) {
        root_block_group_location = 1024 * 2;
    } else {
        std::cerr << "Unsupported block size" << std::endl;
        return;
    }
    ext2_group_desc group_desc;
    file.seekg(root_block_group_location);
    file.read(reinterpret_cast<char *>(&group_desc), sizeof(ext2_group_desc));

    std::cout << "block_size: " << block_size << std::endl;
    std::cout << "bg_block_bitmap: " << group_desc.bg_block_bitmap << std::endl;
    std::cout << "bg_inode_bitmap: " << group_desc.bg_inode_bitmap << std::endl;
    std::cout << "bg_inode_table: " << group_desc.bg_inode_table << std::endl;
    std::cout << "bg_free_blocks_count: " << group_desc.bg_free_blocks_count
              << std::endl;
    std::cout << "bg_free_inodes_count: " << group_desc.bg_free_inodes_count
              << std::endl;
    std::cout << "bg_used_dirs_count: " << group_desc.bg_used_dirs_count
              << std::endl;

    size_t root_inode_table_location = group_desc.bg_inode_table * block_size;
    size_t root_index = (2 - 1) % supablock.s_inodes_per_group;
    size_t root_location_inside_table = root_index * supablock.s_inode_size;
    size_t root_location =
        root_inode_table_location + root_location_inside_table;

    ext2_inode root_entry;
    file.seekg(root_location);
    file.read(reinterpret_cast<char *>(&root_entry), sizeof(ext2_inode));

    if (!(root_entry.i_blocks > 12)) {
        size_t already_read = 0;
        for (size_t i = 0; i < root_entry.i_blocks; i++) {
            size_t already_read_in_block = 0;
            while (already_read < root_entry.i_size &&
                   already_read_in_block < block_size) {
                file.seekg(root_entry.i_block[i] * block_size +
                           already_read_in_block);
                ext2_directory_entry entry;
                file.read(reinterpret_cast<char *>(&entry),
                          sizeof(ext2_directory_entry) - sizeof(entry.name));
                entry.name = new char[entry.name_len + 1];
                file.read(entry.name, entry.name_len);
                entry.name[entry.name_len] = '\0';
                supablock.root_files.push_back(entry);
                already_read += entry.total_len;
                already_read_in_block += entry.total_len;
            }
        }
    }
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
    std::cout << "Root Directory Files:\n";
    std::cout << "Read-only  " << "Hidden  " << "System  " << "Vol Label  "
              << "Long name  " << "Archive  " << "Cluster Num  "
              << "Size           " << "Creation date       "
              << "Modified date        " << "Name" << std::endl;
    for (const auto &file : partition.root_files) {
        print_file_ext2(file, partition);
    }
}

void print_file_ext2(const ext2_directory_entry &file,
                     const ext2_supablock &superblock) {
    std::cout << file.name << std::endl;
}
