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

void print_part(PartitionEntry **p, int type) {
    int i = 0;

    if (type == SUB) {
        printf("\nSubpartition table:\n");
    }
    else {
        printf("\nPartition table:\n");
    }

    printf("%20s", "----Start----");
    printf("%20s", "----End----\n");

    printf("%5s%5s%5s%5s", "Boot", "head", "sec", "cyl");
    printf("%5s%5s%5s%5s%10s%10s\n", 
        "Type", "head", "sec", "cyl", "First", "Size");

    for (i = 0; i < P_TABLE_SIZE; i++) {
        printf("%#5x%5d%5d%5d", 
            p[i]->bootind, p[i]->start_head, 
            p[i]->start_sec, p[i]->start_cyl);
        printf("%#5x%5d%5d%5d%10d%10d\n", 
            p[i]->type, p[i]->end_head, p[i]->end_sec, 
            p[i]->end_cyl, p[i]->lFirst, p[i]->size);
    }
    
    printf("\n");
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

    if (valid_1 == VALID_ONE && valid_2 == VALID_TWO) {
        return 0;
    }

    return 1;
}

void get_partition(Args *args, FILE *f, Part *part) {
    int i;
    PartitionEntry *p1, *p2, *p3, *p4;
    PartitionEntry *sp1, *sp2, *sp3, *sp4;

    PartitionEntry *p_table[] = {p1, p2, p3, p4};
    PartitionEntry *sp_table[] = {sp1, sp2, sp3, sp4};

    for (i = 0; i < P_TABLE_SIZE; i++) {
        p_table[i] = malloc(sizeof(PartitionEntry));
        sp_table[i] = malloc(sizeof(PartitionEntry));
    }

    /* check partition table validity */
    if (partition_invalid(f)) {
        fprintf(stderr, "Invalid partition table\n");
        exit(-1);
    }

    /* navigate to partition table */
    if (fseek(f, PART_TABLE_LOC, SEEK_SET) < 0) {
        perror("fseek failed");
        exit(-1);
    }

    /* read partition table */
    for (i = 0; i < P_TABLE_SIZE; i++) {
        if (fread(p_table[i], sizeof(PartitionEntry), 1, f) < 1) {
            perror("fread failed");
            exit(-1);
        }
    }

    if (args->verbose) {
        print_part(p_table, REG);
    }

    if (p_table[args->part]->bootind != BOOT_MAGIC ||
            p_table[args->part]->type != MINIX_TYPE) {
        fprintf(stderr, "Partition not bootable in Minix\n");
        exit(-1);
    }

    if (args->sub_part != -1) {
        /* go to partition */
        if (fseek(f, p_table[args->part]->lFirst * SECTOR_SIZE, 
                SEEK_SET) < 0) {
            perror("fseek failed");
            exit(-1);
        }

        /* check sub partition table validity */
        if (partition_invalid(f)) {
            fprintf(stderr, "Invalid sub-partition table\n");
            exit(-1);
        }

        /* go to sub partition table */
        if (fseek(f, (p_table[args->part]->lFirst * SECTOR_SIZE) 
                + PART_TABLE_LOC, SEEK_SET) < 0) {
            perror("fseek failed");
            exit(-1);
        }

        /* read subpartition table */
        for (i = 0; i < P_TABLE_SIZE; i++) {
            if (fread(sp_table[i], sizeof(PartitionEntry), 1, f) < 1) {
                perror("fread failed");
                exit(-1);
            }
        }

        if (args->verbose) {
            print_part(sp_table, SUB);
        }

        if (sp_table[args->sub_part]->bootind != BOOT_MAGIC ||
                sp_table[args->sub_part]->type != MINIX_TYPE) {
            fprintf(stderr, "Subpartition not bootable in Minix\n");
            exit(-1);
        }

        part->start = sp_table[args->sub_part]->lFirst * SECTOR_SIZE;
        part->end = part->start + 
            (sp_table[args->sub_part]->size * SECTOR_SIZE);
    }
    else {
        part->start = p_table[args->part]->lFirst * SECTOR_SIZE;
        part->end = part->start + 
            (p_table[args->part]->size * SECTOR_SIZE);
    }

    /* free tables */
    for (i = 0; i < P_TABLE_SIZE; i++) {
        free(p_table[i]);
        free(sp_table[i]);
    }
}

void print_permission(Inode* inode){
    int cnt = 0;
    printf("(");
    while (cnt != 10){
        if ((inode->mode & DIRECTORY) && cnt == 0) {
            printf("d");
            cnt++;
        }
        if ((inode->mode & OWNER_READ) && cnt == 1){
            printf("r");
            cnt++;
        }
        if ((inode->mode & OWNER_WRITE) && cnt == 2){
            printf("w");
            cnt++;
        }
        if ((inode->mode & OWNER_EXECUTE) && cnt == 3){
            printf("x");
            cnt++;
        }
        if ((inode->mode & GROUP_READ) && cnt == 4){
            printf("r");
            cnt++;
        }
        if ((inode->mode & GROUP_WRITE) && cnt == 5){
            printf("w");
            cnt++;
        }
        if ((inode->mode & GROUP_EXECUTE) && cnt == 6){
            printf("x");
            cnt++;
        }
        if ((inode->mode & OTHER_READ) && cnt == 7){
            printf("r");
            cnt++;
        }
        if ((inode->mode & OTHER_WRITE) && cnt == 8){
            printf("w");
            cnt++;
        }
        if ((inode->mode & OTHER_EXECUTE) && cnt == 9){
            printf("x");
            cnt++;
        }
        else {
            printf("-");
            cnt++;
        }
    }
    printf(")\n");
}

void print_time(time_t time){
    char buf[100];
    struct tm  ts;
    ts = *localtime(&time);
    strftime(buf, sizeof(buf), "%a %m %d %H:%M:%S %Y", &ts);
    printf("%s\n", buf);
}

void get_inode(Args *args, FILE *f, SuperBlock *super, Inode *inode){
    int offset = 0;
    if ((offset = fseek(f,  
        (2 + super->i_blocks + super->z_blocks) * super->blocksize, 
        SEEK_SET)) < 0) {
        perror("fseek failed");
        exit(-1);
    }
    fread(inode, sizeof(Inode), 1, f);

    if (args->verbose){
        printf("File inode:\n");
        printf("  uint16_t mode 0x%x ", inode->mode);
        print_permission(inode);
        printf("  uint16_t links %u\n", inode->links);
        printf("  uint16_t uid %u\n", inode->uid);
        printf("  uint16_t gid %u\n", inode->gid);
        printf("  uint32_t size %u\n", inode->size);
        printf("  uint32_t atime %u --- ", inode->atime);
        print_time(inode->atime);
        printf("  uint32_t mtime %u --- ", inode->mtime);
        print_time(inode->mtime);
        printf("  uint32_t ctime %u --- ", inode->ctime);
        print_time(inode->ctime);
    }
    
}

void get_zones(FILE *f, Inode *inode, SuperBlock *superblock){
    int i, j;
    int offset;
    int zonesize =  superblock->blocksize << superblock->log_zone_size;
    Dirent* dirent = malloc(sizeof(dirent));

    printf("  Direct zones:\n");
    for (i = 0; i < DIRECT_ZONES; i++){
        printf("%*s", 10, "");
        printf("zone[%d]  = %8d\n", i, inode->zone[i]);
    }
    printf("  uint32_t  indirect  = %8d\n", inode->indirect);
    printf("  uint32_t double\n");

    for (j = 0; j < DIRECT_ZONES; j++){
        if (inode->zone[j]) {
            offset = inode->zone[j] *  zonesize;
            fseek(f, offset, SEEK_SET);
            for (i = 0; i < zonesize; i += sizeof(Dirent)) {
                fread(dirent, sizeof(Dirent), 1, f);
                printf("dirent inode %d name %s\n", dirent->inode, dirent->d_name);
            }
        }
    }
}

void get_superblock(Args *args, FILE *f, SuperBlock *superblock) {
    int offset = 0;

    if ((offset = fseek(f, SUPER_OFFSET, SEEK_SET)) < 0) {
        perror("fseek failed");
        exit(-1);
    }

    fread(superblock, sizeof(SuperBlock), 1, f);
   
    if (args->verbose) {
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
}


