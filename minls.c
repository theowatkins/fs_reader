#include "min.h"

int main (int argc, char *argv[]) {
    FILE *f;
    Args *args = malloc(sizeof(Args));
    Part part;
    int part_offset;

    get_args(argc, argv, args, LS_FLAG);

    /* open image file for reading */
    if ((f = fopen(args->image_file, "r")) == NULL) {
        perror("fopen failed");
        exit(-1);
    }

    /* get partition info */
    part_offset = get_part_offset(args, f);

    exit(0);
}