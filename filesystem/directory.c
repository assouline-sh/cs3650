#include <string.h>
#include <assert.h>
#include "directory.h"
#include "blocks.h"

// initialize the root directory
void directory_init()
{
    char *working = ".";
    char *parent = "..";

    // allocate an inode for root directory information, its index should be 1 since it is first
    int inode_index = alloc_inode();
    inode_t *inode = get_inode(inode_index);
    inode->mode = 040755; // flag to indicate that this inode is for a directory

    // put the root directories into the allocated inode
    // associating both "." and ".." with the root since there is no designated parent directory
    directory_put(inode, working, inode_index);
    directory_put(inode, parent, inode_index);
}

// return the inode index number of the given directory with the given name
// Args: Directory node to look up, and name of directory we are searching for
// Return -1 on failure, 0 on success
int directory_lookup(inode_t *dd, const char *name)
{
    assert(dd != NULL);
    dirent_t *base = (dirent_t *)blocks_get_block(dd->block);
    // For each entry in the inode, check if the name matches the given name to search for
    dirent_t *dir;
    for (int i = 0; i < dd->size; i++)
    {
        dir = (dirent_t *)(base + i);
        if (strcmp(dir->name, name) == 0)
        {
            // if the name matches, return the inode number
            return dir->inum;
        }
    }
    return -1;
}

// look up the given path from the root directory and return the inode index number
// Args: path of file to look up in the file system
// Return inode index number, or -1 on failure
int tree_lookup(const char *path)
{
    if (strcmp(path, "/") == 0)
    {
        return 0;
    }
    else if (path[0] == '/')
    {
        path++;
    }
    slist_t *exploded = s_explode(path, '/');
    // Conduct a directory lookup on each entry in the 'exploded' path
    int inum = 0;
    while (exploded && strlen(exploded->data) > 0)
    {

        inum = directory_lookup(get_inode(inum), exploded->data);
        if (inum == -1)
        {
            return -1;
        }
        exploded = exploded->next;
    }
    s_free(exploded);
    return inum;
}

// puts a filesystem object into the directory
// Args: directory to place filesystem object in, name of file system object, and its inum
// Return -1 on failure, 0 on success
int directory_put(inode_t *dd, const char *name, int inum)
{
    // create a new directory node and insert the directory name and inum into the directory entry
    dirent_t new_dir;
    strcpy(new_dir.name, name);
    new_dir.inum = inum;
    dirent_t *current_dir = (dirent_t *)(blocks_get_block(dd->block));
    memcpy(current_dir + dd->size, &new_dir, sizeof(dirent_t));
    dd->size += 1; // update size in directory node
    return 0;
}

// delete the file system object with the given name
// Args: directory node to delete from, and name of file system object to delete
// Return -1 on failure, 0 on success
int directory_delete(inode_t *dd, const char *name)
{
    if (strcmp(name, "/") == 0)
    {
        return 1;
    }
    else if (name[0] == '/')
    {
        name++;
    }
    dirent_t *base = (dirent_t *)blocks_get_block(dd->block);
    dirent_t *dir;
    int deleted = -1;
    // for each entry in the inode, check if the name matches the given name
    for (int i = 0; i < dd->size; i++)
    {
        dir = (dirent_t *)(base + i);
        if (strcmp(dir->name, name) == 0)
        {
            // if matches and is not the most recent entry,
            // overwrite with directory from the last slot
            if (dd->size - 1 != i)
            {
                deleted = dir->inum;
                memcpy(dir, base + dd->size - 1, sizeof(dirent_t));
            }
            memset(base + dd->size - 1, 0, sizeof(dirent_t));
            // we only have to decrement the size because if it's the most recent entry, it wont get checked by other
            // functions and will get overwritten when another entry is put in
            dd->size -= 1; // update size of the directory
            if (deleted > 0)
            {
                free_inode(deleted); // free the deleted inode
            }
            return 0;
        }
    }
    return -1;
}

// list all the entries in the given directory path
// Args: file path to list contents of
// Return list on success
slist_t *directory_list(const char *path)
{
    // Get the data block of the given path
    inode_t *node = get_inode(tree_lookup(path));
    dirent_t *base = (dirent_t *)blocks_get_block(node->block);
    dirent_t *dir;

    slist_t *list;
    for (int i = 0; i < node->size; i++)
    {
        dir = (dirent_t *)(base + i);
        list = s_cons(dir->name, list);
    }
    return list;
}

// Print attributes of the directory
// Args: directory node to print
// Return: printed statement
void print_directory(inode_t *dd)
{
    dirent_t *dir = (dirent_t *)blocks_get_block(dd->block);
    for (int i = 1; i < dd->size; i++)
    {
        printf("directory name: %s\ndirectory inum: %d\ndirectory size: %d\n", dir->name, dir->inum, dir->size);
    }
}
