#include "min.h"

int main (int argc, char *argv[]) {
    FILE *f;
    Args *args = malloc(sizeof(Args));
    Part *part = malloc(sizeof(Part));
    SuperBlock* superblock = malloc(sizeof(SuperBlock));
    Inode* root_node = malloc(sizeof(Inode)); 
    Inode* dest_node = malloc(sizeof(Inode)); 
    char *save_path;

    get_args(argc, argv, args, LS_FLAG);

    /* open image file for reading */
    if ((f = fopen(args->image_file, "r")) == NULL) {
        perror("fopen failed");
        exit(EXIT_FAILURE);
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
        /* save path before it's strtoked */
        save_path = malloc(strlen(args->path) + 1);
        memcpy(save_path, args->path, strlen(args->path) + 1);

        /* if path is specified, find file */
        find_file(args, f, superblock, root_node, part, dest_node);

        if (args->verbose) {
            print_inode(dest_node);
        }

        /* check if file is a directory and print accordingly */
        if (dest_node->mode & DIRECTORY) {
            /* print path */
            printf("%s:\n", save_path);
            /* this function prints the directory contents 
               when the file to find is NULL */
            find_in_dir(f, dest_node, superblock, part, NULL, NULL);
        }
        else {
            print_permission(dest_node);
            printf("%10d %s\n", dest_node->size, save_path);
            free(save_path);
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