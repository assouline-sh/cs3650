#define _DEFAULT_SOURCE
#define _BSD_SOURCE 
#define BLOCK_SIZE sizeof(block_t)
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#include <malloc.h> 
#include <stdio.h> 
#include <debug.h>  
#include <assert.h> 
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

// Struct for a block of data
typedef struct block {
  size_t size;		// data size of block
  struct block *next;	// next block in linked list
  int free;		// 0 is free, 1 is not free
} block_t;

// Initialize header of linked list
block_t *head;

// Mutex lock for threads
pthread_mutex_t mutex;


// Whenever two free blocks in the list form a continuous area of memory, merge them into one block
// Args: no arguments
// Return: no return, just coalesce free blocks if possible
void coalesce_free_list() {
  // Search the linked list for two continuous free blocks of memory 
  block_t* block = head;
  while (block != NULL) {
    if (block->free == 0 && block->next != NULL && block->next->free == 0 && block->size + block->next->size <= PAGE_SIZE) {
      block->size += block->next->size;		// coalesce the blocks into one block of combined size
      block->next = block->next->next;		// set next as the next block of the second continuous free block
    }
    block = block->next;
  }
}


// Insert the block at the correct location in the linked list to ensure the list is sorted by address location
// Args: block_t *block - block to insert in the linked list
// Return: no return, inserts block in linked list
void insert_block(block_t *block) {
  if (block > head) {
    block->next = head;
    head = block;
  }
  else {
    block_t *curr = head;
    while (curr->next != NULL && block < curr->next) {
      curr = curr->next;
    }
    block->next = curr->next;
    curr->next = block;
  }
}


// Helper to create a block of data of the given size
// Args: size_t s - how much memory to request from the heap
// Return: block_t - the block of data we have created, with the appropriate fields
block_t *allocate_block(size_t s, size_t num_pages) {
  block_t *block;
  void *request_mem = mmap(NULL, num_pages * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
 
  // Allocating memory succeeded, so initialize the block in the linked list
  // New blocks are added to the front of the list, so the current head becomes the new block's next,
  // and the new block becomes the new head of the linked list
  if (request_mem != MAP_FAILED) {
    block = (block_t *) request_mem;		// setting the block to use the newly freed memory
    block->size = num_pages * PAGE_SIZE;	// number of bytes that were mapped by mmap
    block->free = 1;				// this block is not free
    insert_block(block);			// insert this block at the correct location in the list so it is sorted by address
  }
  // Allocating memory failed, so return NULL instead of a block
  else {
    return NULL;
  }

  // If the block is larger than requested memory size and it is feasible to split it, set up the 
  // leftover as a new memory block and set it as being free
  if (num_pages == 1 && block->size > s && block->size - s >= BLOCK_SIZE + 1) {
    block_t *leftover;
    leftover = (block_t *) ((void *) block + s + BLOCK_SIZE);	// address of leftover block is where data in original block ends	
    leftover->size = block->size - s - BLOCK_SIZE;		// set size to be leftover of original block
    leftover->next = block->next;				// next block is the next block of the original block
    leftover->free = 0;						// this block is free
    block->size = s;						// set original block size to be original requested size
    block->next = leftover;					// set next of original block to be this free block
    coalesce_free_list();					// coalesce free blocks if possible
  }

  // Return selected block's user memory pointer
  return block;
}


// Helper method to find first free fit in linked list, or create new block
// Args: size_t s - how much memory the data block should use
// Return: return a pointer to the address in memory
void *get_block(size_t s) {
  void *ptr;
  block_t *temp = head;
  block_t *block;
  // Check if the requested size is less than a page
  if (s < PAGE_SIZE) {
    // Search the linked list for a free block large enough for the given size
    while (temp != NULL) {				// while you have not reached the end of the linked list
      if (temp->free == 0 && temp->size >= s) {		// check for a free block that has a size equal to or greater than size requesting
        block = temp;
        block->free = 1;
        return (void*) (block + 1);
      }
      temp = temp->next;
    }
    // If a large enough block is not found, use mmap to request a new page and set it up as a block
    block = allocate_block(s, 1);
  }

  // If the requested size is greater than or equal to the page size, compute number of pages needed to satisfy the
  // request, and allocate that many pages with mmap
  else {
    size_t num_pages = (size_t) ceil((s / PAGE_SIZE) + 1);  // 1 instead of block size
    block = allocate_block(s, num_pages);  
  }
  
  // Return the block's user memory pointer 
  assert(block != NULL);
  return (void*) (block + 1);
}


// Allocate memory of the given size
// Args: size_t s - size of data to allocate
// Return: return a pointer to the address of the allocated memory
void *mymalloc(size_t s) {
  assert(s > 0);
  pthread_mutex_lock(&mutex);
  void *ptr = get_block(s);
  pthread_mutex_unlock(&mutex);
  return ptr;
}


// Allocate given size of memory and initialize it to zero
// Args: Number of elements to allocate memory for, and their size
// Return: return a pointer to the address of the allocated memory
void *mycalloc(size_t nmemb, size_t s) {
  assert(s > 0);
  assert(nmemb > 0);
  void *ptr;
  ptr = mymalloc(nmemb * s);		// point to a block of memory of the required size
  assert(ptr != NULL);
  
  pthread_mutex_lock(&mutex);
  memset(ptr, 0, nmemb * s);		// fill entire block of memory with value 0
  debug_printf("Calloc %zu bytes\n", s);
  pthread_mutex_unlock(&mutex);
  return ptr;
}


// Free the pointer
// Args: void *ptr - pointer to the address we want to free
// Return: does not return anything, but sets 'free' flag in block to 0
void myfree(void *ptr) {
  assert(ptr != NULL);
  pthread_mutex_lock(&mutex);
  block_t *temp = (block_t *) ptr - 1;		// get the block the pointer points to
  size_t size = temp->size;
  if (size < PAGE_SIZE) {
    temp->free = 0;				// set the block to be free
    coalesce_free_list();
  }
  else {
    // Find the pointer of the block that points to next
    block_t* prev = head;
    if (prev != temp) {
      while (prev != NULL && prev->next != NULL && prev->next != temp) {
        prev = prev->next;
      }
      // Remove this block from our memory list
      prev->next = temp->next;
    }
    else {
      head = prev->next;
    }

    munmap((void *) temp, size);		// unmap the allocated memory
  }
  debug_printf("Freed %zu bytes\n", size);
  pthread_mutex_unlock(&mutex);
}
