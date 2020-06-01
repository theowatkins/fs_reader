#ifndef H_MIN
#define H_MIN

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define LS_FLAG 0
#define GET_FLAG 1

typedef struct Args {
    uint8_t verbose;
    char *part;
    char *sub_part;
    char *image_file;
    char *path;
    char *dst_path;
} Args;

typedef struct SuperBlock {
  uint32_t s_ninodes;              /* # usable inodes on the minor device */
  uint32_t s_nzones;            /* total device size, including bit maps etc */
  int16_t s_imap_blocks;          /* # of blocks used by inode bit map */
  int16_t s_zmap_blocks;          /* # of blocks used by zone bit map */
  uint16_t s_firstdatazone_old;  /* number of first data zone (small) */
  int16_t s_log_zone_size;        /* log2 of blocks/zone */
  int16_t s_pad;                  /* try to avoid compiler-dependent padding */
  uint32_t s_max_size;             /* maximum file size on this device */
  uint32_t s_zones;               /* number of zones (replaces s_nzones in V2) */
  int16_t s_magic;                /* magic number to recognize super-blocks */
  int16_t s_pad2;                 /* try to avoid compiler-dependent padding */
  uint16_t s_block_size;           /* block size in bytes. */
  uint8_t s_disk_version;          /* filesystem format sub-version */
} SuperBlock;

void get_args(int argc, char **argv, Args *args, int command_type);

void print_usage(int command_type);

#endif