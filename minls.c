#include "min.h"

int main (int argc, char *argv[]) {
    FILE *f;
    Args *args = malloc(sizeof(Args));
    Part *part = malloc(sizeof(Part));
    SuperBlock* superblock = malloc(sizeof(SuperBlock));
    Inode* inode = malloc(sizeof(Inode)); 

    get_args(argc, argv, args, LS_FLAG);

    /* open image file for reading */
    if ((f = fopen(args->image_file, "r")) == NULL) {
        perror("fopen failed");
        exit(-1);
    }

    if (args->part != -1) {
        /* get partition info */
        get_partition(args, f, part);
    }

    get_superblock(args, f, superblock);
    get_inode(args, f, superblock, inode);

    free(args);
    free(part);
    free(superblock);
    free(inode);

    exit(0);
}