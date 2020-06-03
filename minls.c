#include "min.h"

int main (int argc, char *argv[]) {
    FILE *f;
    Args *args = malloc(sizeof(Args));
    Part *part = malloc(sizeof(Part));

    get_args(argc, argv, args, LS_FLAG);

    /* open image file for reading */
    if ((f = fopen(args->image_file, "r")) == NULL) {
        perror("fopen failed");
        exit(-1);
    }

    print_superblock(f);

    if (args->part != -1) {
        /* get partition info */
        get_partition(args, f, part);
    }

    free(args);
    free(part);

    exit(0);
}