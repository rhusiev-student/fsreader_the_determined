#ifndef INCLUDE_EXT2_READER_THE_TRINITARIAN_HPP_
#define INCLUDE_EXT2_READER_THE_TRINITARIAN_HPP_

#include <cstdint>
#include <filesystem>
#include <vector>

#pragma pack(push, 1)
// from presentation
struct _ext2_supablock {          // alignas(16)
    uint32_t s_inodes_count;      /* 0x00: Inodes count */
    uint32_t s_blocks_count;      /* 0x04: Blocks count */
    uint32_t s_r_blocks_count;    /* 0x08: Reserved blocks count */
    uint32_t s_free_blocks_count; /* 0x0C: Free blocks count */
    uint32_t s_free_inodes_count; /* 0x10: Free inodes count */
    uint32_t s_first_data_block;  /* 0x14: First Data Block */
    uint32_t s_log_block_size;   /* 0x18: Block size: log2 (block size ) = 10 */
    int32_t s_log_frag_size;     /* 0x1C: Fragment size (same log2 as above) */
    uint32_t s_blocks_per_group; /* 0x20: Blocks per group */
    uint32_t s_frags_per_group;  /* 0x24: Fragments per group */
    uint32_t s_inodes_per_group; /* 0x28: Inodes per group */
    uint32_t s_mtime;            /* 0x2C: Last mount time, in POSIX time */
    uint32_t s_wtime;            /* 0x30: Last write time, in POSIX time */
    uint16_t s_mnt_count;        /* 0x34: Mount count since last check*/
    int16_t s_max_mnt_count;     /* 0x36: Maximal mount count before a check*/
    uint16_t
        s_magic; /* 0x38: Magic signature : 0xEF53 on disk, (0x53 0xEF LE)*/
    uint16_t
        s_state; /* 0x3A: File system state : 1 == FS OK, 2 == FS has errors */
    uint16_t s_errors;          /* 0x3C: Behaviour when detecting errors :
             1 == ignore, 2 == remount read=only, 3 == kernel panic */
    uint16_t s_minor_rev_level; /* 0x3E: minor revision (version ) level */
    uint32_t s_lastcheck;       /* 0x40: time of last check, in POSIX time */
    uint32_t
        s_checkinterval; /* 0x44: max. time between checks, in POSIX time */
    uint32_t
        s_creator_os; /* 0x48: creator OS ID: eg. 0 == Linux, 3 == FreeBSD */
    uint32_t s_rev_level;  /* 0x4C: Major revision ( version ) level : 0 ==
     original  , 1 == v2 format w/ dynamic inode sizes */
    uint16_t s_def_resuid; /* 0x50: User ID that can use reserved blocks */
    uint16_t s_def_resgid; /* 0x52: Group ID that can use reserved blocks */
    // Below: elds for EXT2_DYNAMIC_REV (V2) superblocks only.
    uint32_t
        s_first_ino; /* 0x54: First non=reserved inode, ver < 1.0: xed as 11
                      */
    uint16_t s_inode_size; /* 0x58: size of inode structure , ver < 1.0: xed as
                           128 */
    uint16_t s_block_group_nr; /* 0x5A: Block group this superblock is part of (
                               if backup copy) */
    uint32_t s_feature_compat; /* 0x5C: Optional features present (not required
    to read or write , mostly == performance=related), for example: 0x01:
    preallocate some blocks for new directory (see byte 0xCD in superblock)
    0x04: FS has journal == is an ext3 FS 0x08: inodes have extended attributes
    0x20: Directories use hash index Many other: https://ext4 . wiki . kernel .
    org/index .php/Ext4_Disk_Layout */
    uint32_t s_feature_incompat;       /* 0x60: Incompatible feature set:
          0x01: Compression. Not implemented.
          0x02: Directory entries record the le type
          Many other. */
    uint32_t s_feature_ro_compat;      /* 0x64: Readonly=compatible feature set:
          0x01: Sparse superblocks == copies of the superblock and
          group descriptors are kept only in the groups whose
          group number is either 0 or a power of 3, 5, or 7.
          0x02: File system uses a 64=bit le size
          0x04: RO_COMPAT_BTREE_DIR, not used
          Many other. */
    uint8_t s_uuid[16];                /* 0x68: 128=bit uuid for volume */
    char s_volume_name[16];            /* 0x78: volume name */
    char s_last_mounted[64];           /* 0x88: directory where last mounted */
    uint32_t s_algorithm_usage_bitmap; /* 0xC8: For compression, not used */
    uint8_t s_prealloc_blocks; /* 0xCC: Nr of blocks to try to preallocate */
    uint8_t s_prealloc_dir_blocks; /* 0xCD: Nr to preallocate for dirs */
    uint16_t s_padding1;
    uint32_t s_reserved[204]; /* Padding to the end of the block */
};

// from presentation
struct ext2_group_desc {
    uint32_t bg_block_bitmap;      /* 0x00: Blocks bitmap block */
    uint32_t bg_inode_bitmap;      /* 0x04: Inodes bitmap block */
    uint32_t bg_inode_table;       /* 0x08: Inodes table block */
    uint16_t bg_free_blocks_count; /* 0x0C: Free blocks count */
    uint16_t bg_free_inodes_count; /* 0x0E: Free inodes count */
    uint16_t bg_used_dirs_count;   /* 0x10: Directories count */
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
};

struct ext2_inode {
    uint16_t i_mode;        /* File mode */
    uint16_t i_uid;         /* Low 16 bits of Owner Uid */
    uint32_t i_size;        /* Size in bytes */
    uint32_t i_atime;       /* Access time */
    uint32_t i_ctime;       /* Creation time */
    uint32_t i_mtime;       /* Modification time */
    uint32_t i_dtime;       /* Deletion Time */
    uint16_t i_gid;         /* Low 16 bits of Group Id */
    uint16_t i_links_count; /* Links count */
    uint32_t i_blocks;      /* Blocks count */
    uint32_t i_flags;       /* File flags */
    uint32_t i_os_specific;
    uint32_t i_block[12]; /* Pointers to blocks */
    uint32_t i_singly_indirect_block;
    uint32_t i_doubly_indirect_block;
    uint32_t i_triply_indirect_block;
    uint32_t i_generation; /* File version (for NFS) */
    uint32_t i_file_acl;   /* File ACL */
    uint32_t i_dir_acl;    /* Directory ACL */
    uint32_t i_faddr;      /* Fragment address */
    uint8_t i_os_specific2[12];
};

struct ext2_directory_entry {
    uint32_t inode;
    uint16_t total_len;
    uint8_t name_len;
    uint8_t file_type;
    char *name;
};

struct ext2_supablock {           // alignas(16)
    uint32_t s_inodes_count;      /* 0x00: Inodes count */
    uint32_t s_blocks_count;      /* 0x04: Blocks count */
    uint32_t s_r_blocks_count;    /* 0x08: Reserved blocks count */
    uint32_t s_free_blocks_count; /* 0x0C: Free blocks count */
    uint32_t s_free_inodes_count; /* 0x10: Free inodes count */
    uint32_t s_first_data_block;  /* 0x14: First Data Block */
    uint32_t s_log_block_size;   /* 0x18: Block size: log2 (block size ) = 10 */
    int32_t s_log_frag_size;     /* 0x1C: Fragment size (same log2 as above) */
    uint32_t s_blocks_per_group; /* 0x20: Blocks per group */
    uint32_t s_frags_per_group;  /* 0x24: Fragments per group */
    uint32_t s_inodes_per_group; /* 0x28: Inodes per group */
    uint32_t s_mtime;            /* 0x2C: Last mount time, in POSIX time */
    uint32_t s_wtime;            /* 0x30: Last write time, in POSIX time */
    uint16_t s_mnt_count;        /* 0x34: Mount count since last check*/
    int16_t s_max_mnt_count;     /* 0x36: Maximal mount count before a check*/
    uint16_t
        s_magic; /* 0x38: Magic signature : 0xEF53 on disk, (0x53 0xEF LE)*/
    uint16_t
        s_state; /* 0x3A: File system state : 1 == FS OK, 2 == FS has errors */
    uint16_t s_errors;          /* 0x3C: Behaviour when detecting errors :
             1 == ignore, 2 == remount read=only, 3 == kernel panic */
    uint16_t s_minor_rev_level; /* 0x3E: minor revision (version ) level */
    uint32_t s_lastcheck;       /* 0x40: time of last check, in POSIX time */
    uint32_t
        s_checkinterval; /* 0x44: max. time between checks, in POSIX time */
    uint32_t
        s_creator_os; /* 0x48: creator OS ID: eg. 0 == Linux, 3 == FreeBSD */
    uint32_t s_rev_level;  /* 0x4C: Major revision ( version ) level : 0 ==
     original  , 1 == v2 format w/ dynamic inode sizes */
    uint16_t s_def_resuid; /* 0x50: User ID that can use reserved blocks */
    uint16_t s_def_resgid; /* 0x52: Group ID that can use reserved blocks */
    // Below: elds for EXT2_DYNAMIC_REV (V2) superblocks only.
    uint32_t
        s_first_ino; /* 0x54: First non=reserved inode, ver < 1.0: xed as 11
                      */
    uint16_t s_inode_size; /* 0x58: size of inode structure , ver < 1.0: xed as
                           128 */
    uint16_t s_block_group_nr; /* 0x5A: Block group this superblock is part of (
                               if backup copy) */
    uint32_t s_feature_compat; /* 0x5C: Optional features present (not required
    to read or write , mostly == performance=related), for example: 0x01:
    preallocate some blocks for new directory (see byte 0xCD in superblock)
    0x04: FS has journal == is an ext3 FS 0x08: inodes have extended attributes
    0x20: Directories use hash index Many other: https://ext4 . wiki . kernel .
    org/index .php/Ext4_Disk_Layout */
    uint32_t s_feature_incompat;       /* 0x60: Incompatible feature set:
          0x01: Compression. Not implemented.
          0x02: Directory entries record the le type
          Many other. */
    uint32_t s_feature_ro_compat;      /* 0x64: Readonly=compatible feature set:
          0x01: Sparse superblocks == copies of the superblock and
          group descriptors are kept only in the groups whose
          group number is either 0 or a power of 3, 5, or 7.
          0x02: File system uses a 64=bit le size
          0x04: RO_COMPAT_BTREE_DIR, not used
          Many other. */
    uint8_t s_uuid[16];                /* 0x68: 128=bit uuid for volume */
    char s_volume_name[16 + 1];        /* 0x78: volume name */
    char s_last_mounted[64 + 1];       /* 0x88: directory where last mounted */
    uint32_t s_algorithm_usage_bitmap; /* 0xC8: For compression, not used */
    uint8_t s_prealloc_blocks; /* 0xCC: Nr of blocks to try to preallocate */
    uint8_t s_prealloc_dir_blocks; /* 0xCD: Nr to preallocate for dirs */
    uint16_t s_padding1;
    std::vector<ext2_directory_entry> root_files;

    ext2_supablock(const _ext2_supablock &_supablock);
};
#pragma pack(pop)

ext2_supablock read_supablock(std::filesystem::path path);
void add_files(ext2_supablock &supablock, std::filesystem::path path);

void print_ext2(ext2_supablock partition);

void print_file_ext2(const ext2_directory_entry &file,
                     const ext2_supablock &superblock);

#endif // INCLUDE_EXT2_READER_THE_TRINITARIAN_HPP_
