#include "min.h"

void get_args(int argc, char **argv, Args *args, int command_type) {
    char opt;

    /* initialize arg struct */
    args->verbose = 0;
    args->part = 0;
    args->sub_part = 0;
    args->image_file = NULL;
    args->path = NULL;
    args->dst_path = NULL;

    while ((opt = getopt(argc, argv, "vp:s:")) != -1) {
        switch (opt) {
            case 'v':
                args->verbose = 1;
                break;
            case 'p':
                //args->part = optarg;
                break;
            case 's':
               // args->sub_part = optarg;
                break;
            default: 
                print_usage(command_type);
                exit(-1);
        }
    }

    if (optind >= argc) {
        print_usage(command_type);
        exit(-1);
    }
    else {
        args->image_file = argv[optind];

        /* get the path (source path for minget) */
        if (optind + 1 < argc) {
            args->path = argv[optind + 1];
        }
        /* source path is required for minget */
        else if (command_type == GET_FLAG) {
            print_usage(command_type);
            exit(-1);
        }

        /* add destination path for minget if provided*/
        if (optind + 2 < argc && command_type == GET_FLAG) {
            args->dst_path = argv[optind + 2];
        }
    }
}

void print_usage(int command_type) {
    if (command_type == LS_FLAG) {
        printf("Usage : minls [ -v ] [ -p part [ -s subpart ] ] imagefile [ path ]\n");
    }
    else {
        printf("Usage : minget [ -v ] [ -p part [ -s subpart ] ] imagefile srcpath [ dstpath ]\n");
    }
}

int partition_invalid(FILE *f, int offset) {
    uint8_t *valid_1, *valid_2;

    if (fseek(f, offset + PART_TABLE_VALID, SEEK_SET) < 0) {
        perror("fseek failed");
        exit(-1);
    }

    fread(valid_1, 1, 1, f);
    fread(valid_2, 1, 1, f);

    if (*valid_1 == VALID_ONE && *valid_2 == VALID_TWO) {
        return 0;
    }

    return 1;
}

int get_part_offset(Args *args, FILE *f) {
    PartitionEntry *part;
    Part *ret;
    int offset = 0;
    char type;

    part = malloc(sizeof(PartitionEntry));
    ret = malloc(sizeof(Part));

  //  if (args->part) {
        /* navigate to partition table */
        if ((offset = fseek(f, PART_TABLE_LOC, SEEK_SET)) < 0) {
            perror("fseek failed");
            exit(-1);
        }

        /* check partition table validity */
     /*   if (partition_invalid(f, offset)) {
            fprintf(stderr, "Invalid partition table\n");
            exit(-1);
        }

        /* navigate to specified partition 
        if ((offset = fseek(f, 
                PART_TABLE_LOC + (sizeof(PartitionEntry) * args->part),
                SEEK_SET)) < 0) {
            perror("fseek failed");
            exit(-1);
        }*/

        /* read partition table entry into my struct */
        fread(part, sizeof(PartitionEntry), 1, f);

        printf("Type %d\n", part->type);

      /*  if (part->type != BOOT_MAGIC) {
            fprintf(stderr, "Invalid type of partition table entry\n");
            exit(-1);
        }*/

        /* return offset of start of partition in bytes */
        return part->lfirst * SECTOR_SIZE;
}
