// Directory manipulation functions.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "blocks.h"
#include "inode.h"
#include "slist.h"


typedef struct diren
{
  char name[DIR_NAME_LENGTH]; // name of the entry
  int inum;                   // index of the inode
  int size;                   // number of entries at this directory
  char _reserved[8];          // padding to make this struct 64 bytes
} dirent_t;

void directory_init();
int directory_lookup(inode_t *dd, const char *name);
int tree_lookup(const char *path);
int directory_put(inode_t *dd, const char *name, int inum);
int directory_delete(inode_t *dd, const char *name);
slist_t *directory_list(const char *path);
void print_directory(inode_t *dd);

#endif
