#ifndef H_MIN
#define H_MIN

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define LS_FLAG 0
#define GET_FLAG 1
#define PART_TABLE_LOC 0x1BE
#define PART_TABLE_VALID 510
#define VALID_ONE 0x55
#define VALID_TWO 0xAA
#define SECTOR_SIZE 512
#define SUB 2
#define REG 1
#define MINIX_TYPE 0x81
#define SUPER_OFFSET 0x400
#define SUPER_MAGIC 0x4D5A
#define DIRECT_ZONES 7
#define P_TABLE_SIZE 4
#define ROOT 1
#define NAME_SIZE 60

#define FILE_TYPE_MASK 0170000 /* File type mask */
#define REG_FILE 0100000 /* Regular file */
#define DIRECTORY 0040000 /* Directory */
#define OWNER_READ 0000400 /* Owner read permission */
#define OWNER_WRITE 0000200 /* Owner write permission */
#define OWNER_EXECUTE 0000100 /* Owner execute permission */
#define GROUP_READ 0000040 /* Group read permission */
#define GROUP_WRITE 0000020 /* Group write permission */
#define GROUP_EXECUTE 0000010  /* Group execute permission */
#define OTHER_READ 0000004 /* Other read permission */
#define OTHER_WRITE 0000002 /* Other write permission */
#define OTHER_EXECUTE 0000001 /* Other execute permission */

typedef struct Args {
    int verbose;
    int part;
    int sub_part;
    char *image_file;
    char *path;
    char *dst_path;
} Args;

typedef struct Part {
    uint32_t start;
    uint32_t end;
} Part;

typedef struct dirent {
  uint32_t inode;
  char  d_name[60];
} Dirent;

typedef struct __attribute__ ((__packed__)) PartitionEntry {
    uint8_t bootind;
    uint8_t start_head;
    uint8_t start_sec;
    uint8_t start_cyl;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_cyl;
    uint32_t lFirst;  
    uint32_t size;
} PartitionEntry;

typedef struct __attribute__ ((__packed__)) Inode {
    uint16_t mode; /* mode */
    uint16_t links; /* number or links */
    uint16_t uid;
    uint16_t gid;
    uint32_t size;
    int32_t atime;
    int32_t mtime;
    int32_t ctime;
    uint32_t zone[DIRECT_ZONES];
    uint32_t indirect;
    uint32_t two_indirect;
    uint32_t unused;
} Inode;

typedef struct __attribute__ ((__packed__)) SuperBlock { 
    uint32_t ninodes; /* number of inodes in this filesystem */
    uint16_t pad1; /* make things line up properly */
    int16_t i_blocks; /* # of blocks used by inode bit map */
    int16_t z_blocks; /* # of blocks used by zone bit map */
    uint16_t firstdata; /* number of first data zone */
    int16_t log_zone_size; /* log2 of blocks per zone */
    int16_t pad2; /* make things line up again */
    uint32_t max_file; /* maximum file size */
    uint32_t zones; /* number of zones on disk */
    int16_t magic; /* magic number */
    int16_t pad3; /* make things line up again */
    uint16_t blocksize; /* block size in bytes */
    uint8_t subversion; /* filesystem sub–version */
} SuperBlock;

void get_args(int argc, char **argv, Args *args, int command_type);

void print_usage(int command_type);

void get_partition(Args *args, FILE *f, Part *part);

void get_root(Args *args, FILE *f, SuperBlock *super, 
                Inode *inode, Part *part);

void get_superblock(Args *args, FILE *f, SuperBlock *superblock, Part *part);

void get_zones(FILE *f, Inode *inode, SuperBlock *superblock);

void find_in_dir(FILE *f, Inode *inode, SuperBlock *super, 
                Part *part, char *find, Inode *dest);

void find_file(Args *args, FILE *f, SuperBlock *super, Inode *root, 
                Part *part, Inode *dest, char *dest_name);

#endif