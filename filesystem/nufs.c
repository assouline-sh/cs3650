// based on cs3650 starter code

#include <assert.h>
// #include <bsd/string.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>
#include "storage.h"
#include "directory.h"

// implementation for: man 2 access
// Checks if a file exists.
// Args: file path to access, mask
// Return -ENOENT on failure, 0 on success
int nufs_access(const char *path, int mask)
{
  if (tree_lookup(path) == -1)
  {
    printf("failed access(%s, %04o) -> %d\n", path, mask, -1);
    return -ENOENT;
  }
  else
  {
    printf("access(%s, %04o) -> %d\n", path, mask, 0);
    return 0;
  }
}

// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
// Args: file path to get attributes of, struct stat to format those values
// Return -ENOENT on failure, 0 on success
int nufs_getattr(const char *path, struct stat *st)
{
  printf("getattr path %s\n", path);
  if (storage_stat(path, st) != 0)
  {
    printf("failed getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, -1, st->st_mode, st->st_size);
    return -ENOENT;
  }
  else
  {
    printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, 0, st->st_mode, st->st_size);
    return 0;
  }
}

// implementation for: man 2 readdir
// lists the contents of a directory
// Return -ENOENT on failure, 0 on success
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  struct stat st;
  int rv;

  rv = nufs_getattr(path, &st);
  assert(rv == 0);
  filler(buf, ".", &st, 0);

  slist_t *list_head = directory_list(path);
  slist_t *list = list_head;
  // iterate through the contents of the directory and get the attributes of each
  while (list)
  {
    char new_path[100];
    strncpy(new_path, path, strlen(path));
    strncat(new_path, list->data, DIR_NAME_LENGTH);
    nufs_getattr(new_path, &st);
    filler(buf, list->data, &st, 0);
    list = list->next;
  }
  s_free(list_head);
  s_free(list);
  printf("readdir(%s) -> %d\n", path, rv);
  return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create function.
// Args: file path to create, mode to set new file to
// Return -ENOENT on failure, 0 on success
int nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
  if (storage_mknod(path, mode, 0) != 0)
  {
    printf("failed mknod(%s, %04o) -> %d\n", path, mode, -1);
    return -ENOENT;
  }
  else
  {
    printf("mknod(%s, %04o) -> %d\n", path, mode, 0);
    return 0;
  }
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
// Args: path of directory to create and mode to set it to
// Return -ENOENT on failure, 0 on success
int nufs_mkdir(const char *path, mode_t mode)
{
  if (storage_mknod(path, mode | 040000, 1) != 0)
  {
    printf("failed mkdir(%s) -> %d\n", path, -1);
    return -ENOENT;
  }
  else
  {
    printf("mkdir(%s) -> %d\n", path, 0);
    return 0;
  }
}

// remove the filesystem object at the given path
// Args: path to unlink
// Return -ENOENT on failure, 0 on success
int nufs_unlink(const char *path)
{
  if (storage_unlink(path) != 0)
  {
    printf("failed rmdir(%s) -> %d\n", path, -1);
    return -ENOENT;
  }
  else
  {
    printf("rmdir(%s) -> %d\n", path, 0);
    return 0;
  }
}

// Remove the directory at the given path
// Args: file path of directory to remove
// Return -ENOENT on failure, 0 on success
int nufs_rmdir(const char *path)
{
  if (nufs_unlink(path) != 0)
  {
    printf("failed rmdir(%s) -> %d\n", path, -1);
    return -ENOENT;
  }
  else
  {
    printf("rmdir(%s) -> %d\n", path, 0);
    return 0;
  }
}

// implements: man 2 rename
// called to move a file within the same filesystem
// Args: file name to rename, and name to rename the file to
// Return -ENOENT on failure, 0 on success
int nufs_rename(const char *from, const char *to)
{
  if (storage_rename(from, to) != 0)
  {
    printf("failed to rename(%s => %s) -> %d\n", from, to, -1);
    return -ENOENT;
  }
  else
  {
    printf("rename(%s => %s) -> %d\n", from, to, 0);
    return 0;
  }
}

// Change the permissions of the given file to the given mode
// Args: file path to change mode of, and mode to change it to
// Return -ENOENT on failure, 0 on success
int nufs_chmod(const char *path, mode_t mode)
{
  if (storage_chmod(path, mode) != 0)
  {
    printf("failed to chmod(%s, %04o) -> %d\n", path, mode, -1);
    return -ENOENT;
  }
  else
  {
    printf("chmod(%s, %04o) -> %d\n", path, mode, 0);
    return 0;
  }
}

// Truncate, or reduce the size, of the given file
// Args: file path to truncate, and size to truncate it to
// Return -ENOENT on failure, 0 on success
int nufs_truncate(const char *path, off_t size)
{
  if (storage_truncate(path, size) != 0)
  {
    printf("failed to truncate(%s, %ld bytes) -> %d\n", path, size, -1);
    return -ENOENT;
  }
  else
  {
    printf("truncate(%s, %ld bytes) -> %d\n", path, size, 0);
    return 0;
  }
}

// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
// Args: file path to open
// Return -ENOENT on failure, 0 on success
int nufs_open(const char *path, struct fuse_file_info *fi)
{
  printf("open(%s) -> %d\n", path, 0);
  return nufs_access(path, 0);
}

// Read data of file at the given path and return the number of characters read
// Args: path of file to read, buffer to read it to, size to read, offset
// Return -ENOENT on failure, 0 on success
int nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  int rv = storage_read(path, buf, size, offset);
  if (rv == -1)
  {
    printf("failed to read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, -1);
    return -ENOENT;
  }
  else
  {
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
  }
}

// Actually write data to the file at the given path
// Args: path of file to write, buffer to write, size of bytes to write, offset
// Return -ENOENT on failure, 0 on success
int nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  int rv = storage_write(path, buf, size, offset);
  if (rv == -1)
  {
    printf("failed to write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return -ENOENT;
  }
  else
  {
    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
  }
}

// Link the node at the 'from' location to the 'to' location
// Implementation was not necessary
int nufs_link(const char *from, const char *to)
{
  int rv = -1;
  printf("link(%s => %s) -> %d\n", from, to, rv);
  return rv;
}

// Update the timestamps on a file or directory.
// Implementation was not necessary
int nufs_utimens(const char *path, const struct timespec ts[2])
{
  int rv = -1;
  printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
  return rv;
}

// Extended operations
// Implementation was not necessary
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data)
{
  int rv = -1;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

// Parse the fuse operations
void nufs_init_ops(struct fuse_operations *ops)
{
  memset(ops, 0, sizeof(struct fuse_operations));
  ops->access = nufs_access;
  ops->getattr = nufs_getattr;
  ops->readdir = nufs_readdir;
  ops->mknod = nufs_mknod;
  // ops->create   = nufs_create; // alternative to mknod
  ops->mkdir = nufs_mkdir;
  ops->link = nufs_link;
  ops->unlink = nufs_unlink;
  ops->rmdir = nufs_rmdir;
  ops->rename = nufs_rename;
  ops->chmod = nufs_chmod;
  ops->truncate = nufs_truncate;
  ops->open = nufs_open;
  ops->read = nufs_read;
  ops->write = nufs_write;
  ops->utimens = nufs_utimens;
  ops->ioctl = nufs_ioctl;
};

struct fuse_operations nufs_ops;

// Mount and run the file system
int main(int argc, char *argv[])
{
  assert(argc > 2 && argc < 6);
  printf("TODO: mount %s as data file\n", argv[--argc]);
  storage_init(argv[argc]);
  nufs_init_ops(&nufs_ops);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
