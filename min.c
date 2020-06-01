#include "min.h"

void get_args(int argc, char **argv, Args *args, int command_type) {
    char opt;

    /* initialize arg struct */
    args->verbose = 0;
    args->part = NULL;
    args->sub_part = NULL;
    args->image_file = NULL;
    args->path = NULL;
    args->dst_path = NULL;

    while ((opt = getopt(argc, argv, "vp:s:")) != -1) {
        switch (opt) {
            case 'v':
                args->verbose = 1;
                break;
            case 'p':
                args->part = optarg;
                break;
            case 's':
                args->sub_part = optarg;
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
