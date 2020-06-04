#include "min.h"

int main (int argc, char *argv[]) {
    FILE *f;
    Args *args = malloc(sizeof(Args));
    Part *part = malloc(sizeof(Part));
    SuperBlock* superblock = malloc(sizeof(SuperBlock));
    Inode* root_node = malloc(sizeof(Inode)); 
    Inode* dest_node = malloc(sizeof(Inode)); 
    char dest_name[NAME_SIZE + 1] = {0};

    get_args(argc, argv, args, LS_FLAG);

    /* open image file for reading */
    if ((f = fopen(args->image_file, "r")) == NULL) {
        perror("fopen failed");
        exit(-1);
    }

    part->start = 0;
    if (args->part != -1) {
        /* get partition info */
        get_partition(args, f, part);
    }

    get_superblock(args, f, superblock, part);

    /* get the root inode */
    get_inode(f, superblock, root_node, part, ROOT);

    if (args->path != NULL) {
        /* if path is specified, find file */
        find_file(args, f, superblock, root_node, part, dest_node, dest_name);

        if (args->verbose) {
            print_inode(dest_node);
        }

        /* print path */
        if(args->path[0] != '/') {
            printf("/");
        }
        printf("%s:\n", args->path);

        /* check if file is a directory and print accordingly */
        if (dest_node->mode & DIRECTORY) {
            /* this function prints the directory contents 
               when the string to find is NULL */
            find_in_dir(f, dest_node, superblock, part, NULL, NULL);
        }
        else {
            print_permission(dest_node);
            printf("%10d %s\n", dest_node->size, dest_name);
        }
    }
    else {
        if (args->verbose) {
            print_inode(root_node);
        }

        /* print root node if no path specified */
        printf("/:\n");
        find_in_dir(f, root_node, superblock, part, NULL, NULL);
    }

    free(args);
    free(part);
    free(superblock);
    free(root_node);
    free(dest_node);

    exit(0);
}