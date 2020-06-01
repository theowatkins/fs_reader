#include "min.h"

int main (int argc, char *argv[]) {
    Args *args = malloc(sizeof(Args));

    get_args(argc, argv, args, GET_FLAG);

    exit(0);
}