/* CPE 453 Assignment 6 */
/* This file contains all of the helper functions for minls.c and minget.c */

#include "min.h"

/* populate args struct based on options/arguments in argv */
void get_args(int argc, char **argv, Args *args, int command_type) {
    char opt;

    /* initialize arg struct */
    args->verbose = 0;
    args->part = -1; /* start as -1 in case part is 0 */
    args->sub_part = -1; /* start as -1 in case sub_part is 0 */
    args->image_file = NULL;
    args->path = NULL;
    args->dst_path = NULL;

    while ((opt = getopt(argc, argv, "vp:s:")) != -1) {
        switch (opt) {
            case 'v': /* verbose */
                args->verbose = 1;
                break;
            case 'p': /* partition */
                args->part = atoi(optarg);
                break;
            case 's': /* subpartition */
                if (args->part == -1) { 
                    /* if no partition is given */
                    print_usage(command_type);
                    exit(EXIT_FAILURE);
                }
                args->sub_part = atoi(optarg);
                break;
            default: 
                print_usage(command_type);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) { /* no args after options */
        print_usage(command_type);
        exit(EXIT_FAILURE);
    }
    else {
        args->image_file = argv[optind];

        /* get the path if given (source path for minget) */
        if (optind + 1 < argc) {
            args->path = argv[optind + 1];
        }
        /* source path is required for minget */
        else if (command_type == GET_FLAG) {
            print_usage(command_type);
            exit(EXIT_FAILURE);
        }

        /* add destination path for minget if provided*/
        if (optind + 2 < argc) {
            args->dst_path = argv[optind + 2];
        }
    }
}

void print_usage(int command_type) {
    if (command_type == LS_FLAG) {
        printf("Usage : minls [ -v ] [ -p part [ -s subpart ] ] ");
        printf("imagefile [ path ]\n");
    }
    else {
        printf("Usage : minget [ -v ] [ -p part [ -s subpart ] ] ");
        printf("imagefile minixpath [ hostpath ]\n");
    }

    printf("\nOptions:\n");
    printf("-p part --- select partition for filesystem (default: none)\n");
    printf("-s sub --- select subpartition for filesystem (default: none)\n");
    printf("-v verbose --- increase verbosity level\n\n");
}

/* prints an entire partition table */
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

/* checks signature of partition and returns 0 if valid, 
   returns 1 if invalid.  File position is expected to 
   already be at the beginning of the partition */
int partition_invalid(FILE *f) {
    uint8_t valid_1, valid_2;

    /* go to signature */
    if (fseek(f, PART_TABLE_VALID, SEEK_CUR) < 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }

    /* read signature */
    if (fread(&valid_1, 1, 1, f) < 1) {
        perror("fread failed");
        exit(EXIT_FAILURE);
    }
    if (fread(&valid_2, 1, 1, f) < 1) {
        perror("fread failed");
        exit(EXIT_FAILURE);
    }

    /* check signature */
    if (valid_1 == VALID_ONE && valid_2 == VALID_TWO) {
        return 0;
    }

    return 1;
}

/* finds partition and subpartition(if given) and populates 
   part with the partition's or subpartition's info */
void get_partition(Args *args, FILE *f, Part *part) {
    int i;
    PartitionEntry *p_table[P_TABLE_SIZE]; /* partition table */
    PartitionEntry *sp_table[P_TABLE_SIZE]; /* subpartition table */

    for (i = 0; i < P_TABLE_SIZE; i++) {
        p_table[i] = malloc(sizeof(PartitionEntry));
        sp_table[i] = malloc(sizeof(PartitionEntry));
    }

    /* check partition table validity */
    if (partition_invalid(f)) {
        fprintf(stderr, "Invalid partition table\n");
        exit(EXIT_FAILURE);
    }

    /* navigate to partition table */
    if (fseek(f, PART_TABLE_LOC, SEEK_SET) < 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }

    /* read partition table */
    for (i = 0; i < P_TABLE_SIZE; i++) {
        if (fread(p_table[i], sizeof(PartitionEntry), 1, f) < 1) {
            perror("fread failed");
            exit(EXIT_FAILURE);
        }
    }

    if (args->verbose) { /* print table */
        print_part(p_table, REG);
    }

    if (p_table[args->part]->type != MINIX_TYPE) { /* check type of table */
        fprintf(stderr, "Partition not bootable in Minix\n");
        exit(EXIT_FAILURE);
    }

    if (args->sub_part != -1) {
        /* go to partition */
        if (fseek(f, p_table[args->part]->lFirst * SECTOR_SIZE, 
                SEEK_SET) < 0) {
            perror("fseek failed");
            exit(EXIT_FAILURE);
        }

        /* check subpartition table validity */
        if (partition_invalid(f)) {
            fprintf(stderr, "Invalid sub-partition table\n");
            exit(EXIT_FAILURE);
        }

        /* go to subpartition table */
        if (fseek(f, (p_table[args->part]->lFirst * SECTOR_SIZE) 
                + PART_TABLE_LOC, SEEK_SET) < 0) {
            perror("fseek failed");
            exit(EXIT_FAILURE);
        }

        /* read subpartition table */
        for (i = 0; i < P_TABLE_SIZE; i++) {
            if (fread(sp_table[i], sizeof(PartitionEntry), 1, f) < 1) {
                perror("fread failed");
                exit(EXIT_FAILURE);
            }
        }

        if (args->verbose) { /* print subpartition table */
            print_part(sp_table, SUB);
        }

        if (sp_table[args->sub_part]->type != MINIX_TYPE) {  
            fprintf(stderr, "Subpartition not bootable in Minix\n");
            exit(EXIT_FAILURE);
        }

        /* populate part with subpartition info */
        part->start = sp_table[args->sub_part]->lFirst * SECTOR_SIZE;
        part->end = part->start + 
            (sp_table[args->sub_part]->size * SECTOR_SIZE);
    }
    else {
        /* populate part with partition info */
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
}

void print_time(time_t time){
    char buf[100];
    struct tm  ts;
    ts = *localtime(&time);
    strftime(buf, sizeof(buf), "%a %m %d %H:%M:%S %Y", &ts);
    printf("%s\n", buf);
}

/* populates inode given inode_num */
void get_inode(FILE *f, SuperBlock *super, Inode *inode, 
                Part * part, int inode_num) {

    int offset = (2 + super->i_blocks + super->z_blocks) * super->blocksize
                + part->start + ((inode_num - 1) * sizeof(Inode));

    /* go to inode in partition */
    if (fseek(f, offset, SEEK_SET) < 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }

    /* read inode */
    if (fread(inode, sizeof(Inode), 1, f) < 0) {
        perror("fread failed");
        exit(EXIT_FAILURE);
    } 
}

void print_inode(Inode *inode) {
    int i;

    printf("\nFile inode:\n");
    printf("  uint16_t mode %*s0x%x ", 9, "", inode->mode);
    printf("(");
    print_permission(inode);
    printf(")\n");
    printf("  uint16_t links %14u\n", inode->links);
    printf("  uint16_t uid %16u\n", inode->uid);
    printf("  uint16_t gid %16u\n", inode->gid);
    printf("  uint32_t size %15u\n", inode->size);
    printf("  uint32_t atime %14u --- ", inode->atime);
    print_time(inode->atime);
    printf("  uint32_t mtime %14u --- ", inode->mtime);
    print_time(inode->mtime);
    printf("  uint32_t ctime %14u --- ", inode->ctime);
    print_time(inode->ctime);
    printf("\n  Direct zones:\n");
    for (i = 0; i < DIRECT_ZONES; i++){
        printf("%12s", "");
        printf("zone[%d]  =%10d\n", i, inode->zone[i]);
    }
    printf("%-12s", "  uint32_t");
    printf("indirect%12d\n", inode->indirect);
    printf("%-12s", "  uint32_t");
    printf("double%14d\n\n", inode->two_indirect);
}

/* finds file named find in directory with inode, 
   placing it's inode in dest if found.
   if find is NULL, prints contents of inode (guaranteed to be directory) */
void find_in_dir(FILE *f, Inode *inode, SuperBlock *super, 
                Part *part, char *find, Inode *dest) {
    int i, j;
    int read_start;
    int read = 0;
    int zonesize =  super->blocksize << super->log_zone_size;
    Dirent* dirent = malloc(sizeof(Dirent));
    Inode *print = malloc(sizeof(Inode));
    
    /* loop through direct zones */
    for (i = 0; i < DIRECT_ZONES; i++) {
        if (inode->zone[i] == 0) { /* hole */
            read += zonesize; /* skip zonesize bytes */
        }
        else {
            /* save zone location */
            read_start = part->start + inode->zone[i] * zonesize;

            /* loop through all directory entries in zone*/
            for (j = 0; j < zonesize / sizeof(Dirent); j++) {

                /* if entire directory has been read */
                if (read >= inode->size) {
                    if(find == NULL) {
                        return;
                    }
                    /* no entry named 'find' in directory */
                    fprintf(stderr, "Invalid path\n");
                    exit(EXIT_FAILURE);
                }

                /* read entry and record number of bytes read */
                if (fseek(f, read_start, SEEK_SET) < 0) {
                    perror("fseek failed");
                    exit(EXIT_FAILURE);
                }
                if (fread(dirent, sizeof(Dirent), 1, f) < 0) {
                    perror("fread failed");
                    exit(EXIT_FAILURE);
                }
                read += sizeof(Dirent);
                read_start += sizeof(Dirent); /* save for next read */

                if (dirent->inode) {
                    if (find == NULL) {
                        /* print entry */
                        get_inode(f, super, print, part, dirent->inode);
                        print_permission(print);
                        printf("%10d %s\n", print->size, dirent->d_name);
                    }
                    else if (strcmp(dirent->d_name, find) == 0) {
                        /* found it, put it in dest */
                        get_inode(f, super, dest, part, dirent->inode);
                        return;
                    }                   
                }
            }
        }
    }
    if (read < inode->size) {
        find_indirect(f, inode, super, part, find, dest, read);
    }
    free(dirent);
    free(print);
}

/* traverses path to find inode of specified file/directory.
   places inode in dest */
void find_file(Args *args, FILE *f, SuperBlock *super, Inode *root, 
                Part *part, Inode *dest) {
    char *find;
    Inode *cur = root; /* start search at root inode */

    find = strtok(args->path, "/");
    
    while (find != NULL) {
        /* search current inode if it's a directory */
        if (cur->mode & DIRECTORY) {
            find_in_dir(f, cur, super, part, find, dest);
            cur = dest; /* directory to search next */
        }
        else {
            fprintf(stderr, "Invalid path\n");
            exit(EXIT_FAILURE);
        }
        find = strtok(NULL, "/"); 
    }
}

/* populates superblock with the superblock info for the partition */
void get_superblock(Args *args, FILE *f, SuperBlock *superblock, Part *part) {

    /* go to superblock */
    if (fseek(f, part->start + SUPER_OFFSET, SEEK_SET) < 0) {
        perror("fseek failed");
        exit(EXIT_FAILURE);
    }

    /* read superblock */
    if (fread(superblock, sizeof(SuperBlock), 1, f) < 0) {
        perror("fread failed");
        exit(EXIT_FAILURE);
    }

    /* check magic number */
    if (superblock->magic != SUPER_MAGIC) {
        fprintf(stderr, "Bad magic number. (0x%04x)\n", superblock->magic);
        fprintf(stderr, "This doesn't look like a MINIX filesystem.\n");
        exit(EXIT_FAILURE);
    }

    if (args->verbose) { /* print superblock */
        printf("\nSuperblock Contents:\nStored Fields:\n");
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

/* writes data FROM file with src as its inode TO dst_fp 
   (or stdout if dst_fp is NULL) */
void copy_data(FILE *f, FILE *dst_fp, Inode* src, SuperBlock *superblock,
                Part *part){
    int i, j, offset, buf_size;
    int zones = superblock->blocksize / sizeof(src->indirect);
    uint32_t zone_num;
    int zonesize =  superblock->blocksize << superblock->log_zone_size;
    Buffer *buffer;
    Buffer buf; /* buffer for storing the file's data */

    buf.buf = malloc(src->size); /* data */
    buf.rem_size = src->size; /* remaining size */
    buf.buf_offset = 0; /* bytes read so far */

    i = j = offset = buf_size = 0;
    buffer = &buf;

    while(buffer->rem_size > 0 && i < DIRECT_ZONES) {
        /* loop through direct zones */
        if (src->zone[i]) {
            offset = part->start + src->zone[i] * zonesize;
            if (fseek(f, offset, SEEK_SET) < 0) {
                perror("fseek failed");
                exit(EXIT_FAILURE);
            }
            /* read entire zone or remaining bytes (if l.t. zonesize) */
            buf_size = buffer->rem_size < zonesize ?  
                buffer->rem_size : zonesize;
            if (fread(buffer->buf + buffer->buf_offset, buf_size, 1, f) < 0) {
                perror("fread failed");
                exit(EXIT_FAILURE);
            }
            buffer->buf_offset += buf_size;
            buffer->rem_size -= buf_size;
        }
        else { /* hole */
            buf_size = buffer->rem_size < zonesize ?  
                buffer->rem_size : zonesize;
            memset(buffer->buf + buffer->buf_offset, 0, buf_size);
            buffer->buf_offset += buf_size;
            buffer->rem_size -= buf_size;
        }
        i++;
    }
    
    if (buffer->rem_size > 0){
        /* get data from indirect block */
        copy_indirect(f, buffer, superblock, src->indirect, part);
    }

    if (src->two_indirect) {
        /* get data from double indirect block */
        offset = part->start + src->two_indirect * zonesize;

        while(buffer->rem_size > 0 && j < zones){
            if (fseek(f, offset, SEEK_SET) < 0) {
                perror("fseek failed");
                exit(EXIT_FAILURE);
            }

            /* Get indirect zone number in the double indirect block */
            if (fread(&zone_num, sizeof(src->two_indirect), 1, f) < 0) {
                perror("fread failed");
                exit(EXIT_FAILURE);
            }
            offset += sizeof(src->two_indirect); 
            j++;

            if (zone_num) 
                copy_indirect(f, buffer, superblock, zone_num, part);
            else { /* hole, put 0s in buffer */
                buf_size = buffer->rem_size < zones * zonesize ?  
                    buffer->rem_size : zones * zonesize;
                memset(buffer->buf + buffer->buf_offset, 0, buf_size);
                buffer->buf_offset += buf_size;
                buffer->rem_size-=buf_size;
            }
        }
    }
    else {
        /* rest of file is a hole */
        memset(buffer->buf + buffer->buf_offset, 0, buffer->rem_size);
    }

    if (dst_fp){
        /* write to destination file */
        if (fwrite(buffer->buf, src->size, 1, dst_fp) < 0) {
            perror("fwrite failed");
            exit(EXIT_FAILURE);
        }
    }
    else {
        /* if no destination given, write to stdout */
        for (i = 0; i < src->size; i++)
            printf("%c", buffer->buf[i]);
    }
    free(buffer->buf);
}

/* given zone_start (the zone number of the indirect block) 
   read as much data as possible into the buffer */
void copy_indirect(FILE *f, Buffer *buffer, SuperBlock *superblock, 
                    uint32_t zone_start, Part *part) {
    int buf_size, zone_offset, j;
    uint32_t zone_num;
    int zonesize =  superblock->blocksize << superblock->log_zone_size;
    int offset = part->start + zone_start * zonesize;
    int zones = superblock->blocksize / sizeof(zone_start); 

    buf_size = zone_offset = j = 0;

    if(!zone_start) {
        /* entire indirect zone is a hole */
        buf_size = buffer->rem_size < zonesize * zones ? 
            buffer->rem_size : zonesize * zones;
        memset(buffer->buf + buffer->buf_offset, 0, buf_size);
        buffer->buf_offset += buf_size;
        buffer->rem_size -= buf_size;
        return;
    }

    while(buffer->rem_size > 0 && j < zones) {
        if (fseek(f, offset, SEEK_SET) < 0) {
            perror("fseek failed");
            exit(EXIT_FAILURE);
        }

        /* Get zone number at current offset in the indirect block */
        if (fread(&zone_num, sizeof(zone_start), 1, f) < 0) {
            perror("fread failed");
            exit(EXIT_FAILURE);
        }
        offset += sizeof(zone_start); 
        j++; 

        if (zone_num) {
            /* go to zone */
            zone_offset = part->start +  zone_num * zonesize;
            if (fseek(f, zone_offset, SEEK_SET) < 0) {
                perror("fseek failed");
                exit(EXIT_FAILURE);
            }

            /* read data into buffer */
            buf_size = buffer->rem_size < zonesize ?  
                buffer->rem_size : zonesize;
            if (fread(buffer->buf + buffer->buf_offset, buf_size, 1, f) < 0) {
                perror("fread failed");
                exit(EXIT_FAILURE);
            }
            buffer->buf_offset += buf_size;
            buffer->rem_size-=buf_size;
        }
        else { /* hole, put 0s in buffer */
            buf_size = buffer->rem_size < zonesize ?  
                buffer->rem_size : zonesize;
            memset(buffer->buf + buffer->buf_offset, 0, buf_size);
            buffer->buf_offset += buf_size;
            buffer->rem_size-=buf_size;
        }
    } 
}

/* finds file named find in directory (inode) if not found in direct zones, 
   placing the file's inode in dest. 
   if find is NULL, prints directory contents. */
void find_indirect(FILE *f, Inode *inode, SuperBlock *super, 
                Part *part, char *find, Inode *dest, int read) {
    int i, j, read_start, zone_offset;
    uint32_t zone_num;
    int zonesize =  super->blocksize << super->log_zone_size;
    Dirent* dirent = malloc(sizeof(Dirent)); /* entry in directory */
    Inode *print = malloc(sizeof(Inode)); /* inode of entry for printing */
    int zones = super->blocksize / sizeof(inode->indirect);
    read_start =  part->start + inode->indirect *  zonesize; 

    for (i = 0; i < zones; i++) {
        if (fseek(f, read_start, SEEK_SET) < 0) {
            perror("fseek failed");
            exit(EXIT_FAILURE);
        }
        read_start += sizeof(inode->indirect); 
        /* Get zone number in the indirect block */
        if (fread(&zone_num, sizeof(inode->indirect), 1, f) < 0) {
            perror("fread failed");
            exit(EXIT_FAILURE);
        }
        zone_offset = part->start +  zone_num *  zonesize; 
        /* search zone if non empty else advance to next zone */
        if (zone_num) {
            for (j = 0; j < zonesize / sizeof(Dirent); j++) {

                /* if entire directory has been read */
                if (read >= inode->size) {
                    if(find == NULL) {
                        return;
                    }
                    /* no entry named 'find' in directory */
                    fprintf(stderr, "Invalid path\n");
                    exit(EXIT_FAILURE);
                }

                /* read entry and record number of bytes read */
                if (fseek(f, zone_offset, SEEK_SET) < 0) {
                    perror("fseek failed");
                    exit(EXIT_FAILURE);
                }
                if (fread(dirent, sizeof(Dirent), 1, f) < 0) {
                    perror("fread failed");
                    exit(EXIT_FAILURE);
                }
                read += sizeof(Dirent);
                zone_offset += sizeof(Dirent); /* save for next read */

                if (dirent->inode != 0) {
                    if (find == NULL) {
                        /* print every entry */
                        get_inode(f, super, print, part, dirent->inode);
                        print_permission(print);
                        printf("%10d %s\n", print->size, dirent->d_name);
                    }
                    else if (strcmp(dirent->d_name, find) == 0) {
                        /* found it, put it in dest */
                        get_inode(f, super, dest, part, dirent->inode);
                        return;
                    }                   
                }
            }
        }
    }
    free(dirent);
    free(print);

    return;
}
