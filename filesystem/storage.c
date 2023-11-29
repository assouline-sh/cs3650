#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#include "slist.h"
#include "directory.h"
#include "storage.h"
#include "inode.h"
#include "blocks.h"
#include "bitmap.h"

// Initialize storage for the file at the given path
// Args: path to initialize
void storage_init(const char *path)
{
    // Load and initialize the given disk image, block 0 stores the block bitmap and the inode bitmap
    blocks_init(path);
    // Initialize the root directory if it has not already been initialized
    void *ibm = get_blocks_bitmap();
    if (bitmap_get(ibm, 1) == 0)
    {
        directory_init();
    }
}

// Get the inode's attributes
// Args: path of the file to get its attributes, and struct to return those values
// Return: -1 on failure, 0 on success
int storage_stat(const char *path, struct stat *st)
{
    // Split the given path into the nested directory path and the target file system object
    char nested_dir_path[100];
    strcpy(nested_dir_path, path);
    char *slash;
    if (slash = strrchr(nested_dir_path, '/'))
    {
        *slash = '\0';
    }
    char nested_node[100] = "/";
    strcpy(nested_node, (char *)(strrchr(path, '/')));
    
    // Get the inode index at the given path, then get the inode pointer
    int inum = tree_lookup(path);
    inode_t *node = get_inode(inum);
    if (node == NULL || inum == -1)
    {
        return -1;
    }

    // Assign the inode's values to the attributes of struct stat
    memset(st, 0, sizeof(struct stat));
    st->st_nlink = node->refs;
    st->st_mode = node->mode;
    st->st_size = node->size;
    st->st_ino = node->inum;
    st->st_uid = getuid();
    return 0;
}

// make a filesystem object like a file or directory
// Args: file path to create, mode to set new file at, and directory to place it in
// Return: -1 on failure, 0 on success
int storage_mknod(const char *path, int mode, int directory)
{
    // remove the first / from the path
    char new_path[100];
    strcpy(new_path, &path[1]);

    // split the path into the nested directory path and the target file system object
    char nested_dir_path[100];
    strcpy(nested_dir_path, path);
    char *slash;
    if (slash = strrchr(nested_dir_path, '/'))
    {
        *slash = '\0';
    }
    char nested_node[100] = "/";
    strcpy(nested_node, (char *)(strrchr(path, '/')) + 1);
    
    // allocate a new inode and set to the given mode
    int inum = alloc_inode();
    inode_t *inode = get_inode(inum);
    if (directory == 1)
    {
        inode->mode = 040755;
    }
    else
    {
        inode->mode = 0100664;
    }
    
    // add the new inode to the directory at the given path, starting from the root directory
    if (strlen(nested_dir_path) == 0)
    {
	// If we are putting the node at the root directory
        if (directory_put(get_inode(0), nested_node, inum) != 0)
        {
            return -1;
        }
    }
    else
    {
	// If we are putting the node in a nested directory
        int dir_inum = tree_lookup(nested_dir_path);
        inode_t *nested_dir_inode = get_inode(dir_inum);
        if (directory_put(nested_dir_inode, nested_node, inum) != 0)
        {
            return -1;
        }
    }
    return 0;
}

// remove the file at the given path
// Args: path of file to remove
// Return: -1 on failure, 0 on success
int storage_unlink(const char *path)
{
    // get the inode and free it
    int inum = tree_lookup(path);
    inode_t *node = get_inode(inum);
    if (node == NULL || inum == -1)
    {
        printf("get inode fail\n");
        return -1;
    }

    // if it is a directory, check if it's empty before deleting
    if (node->mode == 040755)
    {
        if (node->size > 0)
        {
            printf("dir not empty\n");
            return -1;
        }
    }
    free_inode(inum);

    // remove the node from the directory
    char parent_path[100] = "";
    char filename[100] = "";
    slist_t *list = s_explode(path, '/');
    if (list->next && list->next->next)
    {
	// Iterate through the 'exploded' path
        while (list->next)
        {
            int len = strlen(list->data);
            if (len > 0)
            {
                strncat(parent_path, list->data, len);
            }
            list = list->next;
            strcpy(filename, list->data);
        }
    }
    else
    {
        strcpy(parent_path, "/");
        strcpy(filename, path);
    }

    // remove the node from the directory
    int dir_inum = tree_lookup(parent_path);
    int dd = directory_delete(get_inode(dir_inum), filename);
    s_free(list);
    if (dd == -1)
    {
        return -1;
    }

    return 0;
}

// Rename the given file system object at the 'from' location to the 'to' location
// Also used when moving files between directories
// Args: file name to rename, and name to rename the file to
// Return: -1 on failure, 0 on success
int storage_rename(const char *from, const char *to)
{
    char path[100] = "";     // the path of the directory the file is in
    char to_path[100] = "";  // the path of the directory to be moved to
    char filename[100] = ""; // the name of the file/directory being moved
    // get the file directory to get the file from
    slist_t *list = s_explode(from, '/');
    if (list->next && list->next->next)
    {
	// Iterate through the 'exploded' path and 
        while (list->next)
        {
            int len = strlen(list->data);
            if (len > 0)
            {
                strncat(path, list->data, len);
            }
            list = list->next;
        }
    }
    else
    {
        strcpy(path, "/");
    }

    s_free(list);

    // get the to file directory to put into
    slist_t *to_list = s_explode(to, '/');
    if (to_list->next)
    {
        while (to_list->next)
        {
            int len = strlen(to_list->data);
            if (len > 0)
            {
                strncat(to_path, to_list->data, len);
            }
            to_list = to_list->next;
        }
    }
    else
    {
        strcpy(to_path, "/");
    }
    strcpy(filename, to_list->data);
    s_free(to_list);
    
    // search for the directory the file is in
    int from_inum = tree_lookup(path);
    int to_inum = tree_lookup(to_path);
    if (from_inum == -1 || to_inum == -1)
    {
        return -1;
    }
    inode_t *from_inode = get_inode(from_inum);
    inode_t *to_inode = get_inode(to_inum);

    // add the new path to the directory
    directory_put(to_inode, filename, tree_lookup(from));
    // remove the old path from the directory
    directory_delete(from_inode, filename);
}

// Change the permission mode for the given file
// Args: file path to change mode for, and the mode to change it to
// Return: -1 on failure, 0 on success
int storage_chmod(const char *path, mode_t mode)
{
    // Get the inode index at the given path, then get the inode pointer
    int inum = tree_lookup(path);
    inode_t *node = get_inode(inum);
    if (node == NULL || inum == -1)
    {
        return -1;
    }
    else
    {
        node->mode = mode;
        return 0;
    }
}

// Truncate, or reduce the size, of the given file
// Args: file path to truncate, and the size the resulting file should be
// Return: -1 on failure, 0 on success
int storage_truncate(const char *path, off_t size)
{
    // Get the inode index at the given path, then get the inode pointer
    int inum = tree_lookup(path);
    inode_t *node = get_inode(inum);
    if (node == NULL || inum == -1)
    {
        return -1;
    }
    else
    {
        node->size = size;
        return 0;
    }
}

// Reads data into the buffer and returns number of characters read
// Args: path of the file to read, buffer to copy into, size of data to read, and offset
// Return: -1 on failure, number of bytes read on success
int storage_read(const char *path, char *buf, size_t size, off_t offset)
{
    // Get the inode index at the given path, then get the inode pointer
    int inum = tree_lookup(path);
    inode_t *node = get_inode(inum);
    if (node == NULL || inum == -1)
    {
        return -1;
    }
    // Get the pointer of the start of the corresponding data block, and increase by offset
    void *start_addr = blocks_get_block(node->block);
    start_addr += offset % BLOCK_SIZE;

    memcpy(buf, start_addr, node->size);
    printf("printing buffer: \n %s \n", buf);
    return strlen(buf);
}

// Writes data from the buffer into memory
// Args: path of file to write, buffer to write from, size of data to write, and offset
// Return: -1 on failure, number of bytes written
int storage_write(const char *path, const char *buf, size_t size, off_t offset)
{
    // Get the inode index at the given path, then get the inode pointer
    int inum = tree_lookup(path);
    inode_t *node = get_inode(inum);
    if (node == NULL || inum == -1)
    {
        return -1;
    }
    // Get the pointer of the start of the corresponding data block, and increase by offset
    void *start_addr = blocks_get_block(node->block);
    start_addr += offset % BLOCK_SIZE;
    node->size = size; // update node size to # of bytes in the file

    size_t len = strlen(buf);
    // Ensure that the number of bytes written does not exceed the size of data
    if (offset < size)
    {
        if (offset + size > len)
        {
            size = len - offset;
        }

        memcpy(start_addr, buf, size);
        char *written = (char *)start_addr;
        printf("written text : \n %s \n", written);
    }
    else
    {
        size = 0;
    }
    return size;
}
