#include "inode.h"
#include "bitmap.h"
#include "blocks.h"
#include <sys/stat.h>

// Print the attributes of the given node
// Args: inode to print
// Return: printed inode attributes
void print_inode(inode_t *node)
{
    printf("inode refs: %d\ninode mode: %d\ninode size: %d\ninode block: %d\ninode inum: %d\n", node->refs, node->mode, node->size, node->block, node->inum);
}

// Get the inode at the given index
// Args: index number of a node
// Return: pointer to node with given index number
inode_t *get_inode(int inum)
{
    if (inum == -1)
    {
        return NULL;
    }
    // Get inode table block and increment pointer by size of inodes until given inum
    inode_t *nodes = get_inode_bitmap() + BLOCK_BITMAP_SIZE;
    return &nodes[inum];
}

// Allocate a new inode and return its index
int alloc_inode()
{
    // Return a pointer to the beginning of the inode table bitmap
    void *ibm = get_inode_bitmap();

    // Iterate through inode table bitmap until free inode
    for (int ii = 0; ii < BLOCK_COUNT; ++ii)
    {
        if (!bitmap_get(ibm, ii))
        {
            // Initialize a new inode
            inode_t *inode = get_inode(ii);
            inode->refs = 1;
            inode->mode = S_IFDIR;
            inode->size = 0;
            inode->block = alloc_block();
            inode->inum = ii;

            // Set the corresponding bit in the bitmap to 1 (not free)
            bitmap_put(ibm, ii, 1);
            printf("+ alloc_inode() -> %d\n", ii);
            return ii;
        }
    }
    return -1;
}

// Deallocate the inode with the given index
// Args: index number of node to free
void free_inode(int inum)
{
    printf("+ free_inode(%d)\n", inum);
    // Free the associated data block
    inode_t *node = get_inode(inum);
    free_block(node->block);

    // Set the bit to 0, or free
    void *ibm = get_inode_bitmap();
    bitmap_put(ibm, inum, 0);
}

