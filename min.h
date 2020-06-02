#ifndef H_MIN
#define H_MIN

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define LS_FLAG 0
#define GET_FLAG 1
#define PART_TABLE_LOC 0x1BE
#define PART_TABLE_VALID 510
#define VALID_ONE 0x55
#define VALID_TWO 0xAA
#define SECTOR_SIZE 512
#define BOOT_MAGIC 0x80
#define MINIX_TYPE 0x81
#define SUPER_OFFSET 0x400
#define DIRECT_ZONES 7

typedef struct Args {
    uint8_t verbose;
    uint8_t part;
    uint8_t sub_part;
    char *image_file;
    char *path;
    char *dst_path;
} Args;

typedef struct Part {
    int start;
    int end;
} Part;

typedef struct __attribute__ ((__packed__)) PartitionEntry {
    uint8_t bootind;
    uint8_t start_head;
    uint8_t start_sec;
    uint8_t start_cyl;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_cyl;
    uint32_t lfirst;  
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

int get_part_offset(Args *args, FILE *f);

void print_superblock(FILE *f);

#endif