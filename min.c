#include "min.h"

void get_args(int argc, char **argv, Args *args, int command_type) {
    char opt;

    /* initialize arg struct */
    args->verbose = 0;
    args->part = -1; /* start as -1 in case part/sub_part is 0 */
    args->sub_part = -1;
    args->image_file = NULL;
    args->path = NULL;
    args->dst_path = NULL;

    while ((opt = getopt(argc, argv, "vp:s:")) != -1) {
        switch (opt) {
            case 'v':
                args->verbose = 1;
                break;
            case 'p':
                args->part = atoi(optarg);
                break;
            case 's':
                if (args->part == -1) {
                    print_usage(command_type);
                    exit(-1);
                }
                args->sub_part = atoi(optarg);
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

    printf("\nOptions:\n");
    printf("-p part --- select partition for filesystem (default: none)\n");
    printf("-s sub --- select subpartition for filesystem (default: none)\n");
    printf("-v verbose --- increase verbosity level\n\n");
}

void print_part(PartitionEntry *p, int type) {
    if (type == SUB) {
        printf("\nSub-partition table:\n");
    }
    else {
        printf("\nPartition table\n");
    }

    printf("\ttype: 0x%x\n", p->type);
    printf("\tlFirst: %d\n", p->lFirst);
    printf("\tsize: %d\n\n", p->size);
}

int partition_invalid(FILE *f) {
    uint8_t valid_1, valid_2;

    /* head should be at beginning of disk or outer partition */
    if (fseek(f, PART_TABLE_VALID, SEEK_CUR) < 0) {
        perror("fseek failed");
        exit(-1);
    }

    /* read signature */
    if (fread(&valid_1, 1, 1, f) < 1) {
        perror("fread failed");
        exit(-1);
    }
    if (fread(&valid_2, 1, 1, f) < 1) {
        perror("fread failed");
        exit(-1);
    }

    if (*valid_1 == VALID_ONE && *valid_2 == VALID_TWO) {
        return 0;
    }

    return 1;
}

void get_partition(Args *args, FILE *f, Part *part) {
    PartitionEntry *part_entry, *sub_part;

    part_entry = malloc(sizeof(PartitionEntry));
    sub_part = malloc(sizeof(PartitionEntry));

    /* check partition table validity */
    if (partition_invalid(f)) {
        fprintf(stderr, "Invalid partition table\n");
        exit(-1);
    }

    /* navigate to specified partition */
    if (fseek(f, PART_TABLE_LOC + (sizeof(PartitionEntry) * args->part),
                SEEK_SET) < 0) {
        perror("fseek failed");
        exit(-1);
    }

    /* read partition table entry into my struct */
    if (fread(part_entry, sizeof(PartitionEntry), 1, f) < 1) {
        perror("fread failed");
        exit(-1);
    }

    if (part_entry->type != BOOT_MAGIC) {
        fprintf(stderr, "Invalid type of partition table entry\n");
        exit(-1);
    }

    if (args->verbose) {
        print_part(part_entry, REG);
    }

    if (args->sub_part != -1) {
        /* go to partition */
        if (fseek(f, part_entry->lFirst * SECTOR_SIZE, SEEK_SET) < 0) {
            perror("fseek failed");
            exit(-1);
        }

        /* check sub partition table validity */
        if (partition_invalid(f)) {
            fprintf(stderr, "Invalid sub-partition table\n");
            exit(-1);
        }

        /* go to sub partition entry */
        if (fseek(f, (part_entry->lFirst * SECTOR_SIZE) + PART_TABLE_LOC + 
                (sizeof(PartitionEntry) * args->sub_part), SEEK_SET) < 0) {
            perror("fseek failed");
            exit(-1);
        }

        /* read partition table entry into my struct */
        if (fread(part_entry, sizeof(PartitionEntry), 1, f) < 1) {
            perror("fread failed");
            exit(-1);
        }

        if (args->verbose) {
            print_part(part_entry, SUB);
        }
    }

    part->start = part_entry->lFirst * SECTOR_SIZE;
    part->end = part->start + (part_entry->size * SECTOR_SIZE);

    free(part_entry);
    free(sub_part);
}

#include "min.h"

void print_inodes(FILE *f, SuperBlock *super){
   int offset = 0;
   if ((offset = fseek(f, SUPER_OFFSET + 
   (super->blocksize * (super->i_blocks + super->z_blocks))
   , SEEK_SET)) < 0) {
      perror("fseek failed");
      exit(-1);
   }
}

void print_superblock(FILE *f) {
   int offset = 0;
   SuperBlock* superblock = malloc(sizeof(SuperBlock));

   if ((offset = fseek(f, SUPER_OFFSET, SEEK_SET)) < 0) {
      perror("fseek failed");
      exit(-1);
   }
   

   fread(superblock, sizeof(SuperBlock), 1, f);
   printf("Superblock Contents:\nStored Fields:\n");
   printf("  ninodes %12u\n", superblock->ninodes);
   printf("  i_blocks %11d\n", superblock->i_blocks);
   printf("  z_blocks %11d\n", superblock->z_blocks);
   printf("  firstdata %10u\n", superblock->firstdata);
   printf("  log_zone_size %6d (zone size: %d)\n", 
   superblock->log_zone_size, 
   superblock->blocksize << superblock->log_zone_size);
   printf("  max_file %11u\n", superblock->max_file);
   printf("  magic%*s0x%x\n", 9, "", superblock->magic);
   printf("  zones %14d\n", superblock->zones);
   printf("  blocksize %10u\n", superblock->blocksize);
   printf("  subversion %9u\n", superblock->subversion);
}


