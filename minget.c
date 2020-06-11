/* CPE 453 Assignment 6 */
/* this file contains the main for the minget command */
#include "min.h"

int main (int argc, char *argv[]) {
   FILE *f;
   FILE *dst_fp = NULL;
   Args *args = malloc(sizeof(Args));
   Part *part = malloc(sizeof(Part));
   SuperBlock* superblock = malloc(sizeof(SuperBlock));
   Inode* root_node = malloc(sizeof(Inode)); 
   Inode* from_node = malloc(sizeof(Inode)); 

   get_args(argc, argv, args, GET_FLAG); /* populate args */

   /* open image file for reading */
   if ((f = fopen(args->image_file, "r")) == NULL) {
      printf("Unable to open disk image \"%s\".\n", args->image_file);
      exit(EXIT_FAILURE);
   }

   part->start = 0;
   if (args->part != -1) {
      /* get partition info */
      get_partition(args, f, part);
   }

   get_superblock(args, f, superblock, part); /* populate superblock */

   /* get the root inode */
   get_inode(f, superblock, root_node, part, ROOT);

   /* find file given in source path */
   find_file(args, f, superblock, root_node, part, from_node);
   if (args->verbose) {
      print_inode(from_node);
   }

   /* check if file is a regular file */
   if ((from_node->mode & FILE_TYPE_MASK) != REG_FILE) {
      printf("%s: Not a regular file. \n", args->path);
      exit(EXIT_FAILURE);
   }

   /* open file for writing in destination path */
   if (args->dst_path) {
      dst_fp = fopen(args->dst_path,"w");
      if(dst_fp == NULL) {
         perror("wrong destination path");   
         exit(EXIT_FAILURE);             
      }
   }

   copy_data(f, dst_fp, from_node, superblock, part);

   free(args);
   free(part);
   free(superblock);
   free(root_node);
   free(from_node);

   exit(0);
}
