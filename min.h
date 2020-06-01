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

void get_args(int argc, char **argv, Args *args, int command_type);

void print_usage(int command_type);

#endif