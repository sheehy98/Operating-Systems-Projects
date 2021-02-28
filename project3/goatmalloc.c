#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "goatmalloc.h"

static void *_arena_start;
size_t _arena_size;
int statusno;
FILE *fp;

// use OS commands (start and ending bytes from OS)
// after make sure it's a multiple of the page size
// allocate space in memory of type __node_t with size adjusted size

int init(size_t size)
{
  // fp = freopen("output.txt", "w", stdout);
  _arena_start = 0;
  statusno = 0;

  printf("Initializing arena:\n");
  size_t pageSize = getpagesize();
  printf("...requested size %d bytes\n", (signed int)size);

  if ((int)size < 0 || (int)size > MAX_ARENA_SIZE) // CHANGED THIS TO MATCH REQUIREMENTS
  {
    // Invalid size
    printf("...error: requested size larger than MAX_ARENA_SIZE (%d)\n", MAX_ARENA_SIZE);
    statusno = ERR_BAD_ARGUMENTS;
    return statusno;
  }

  printf("...pagesize is %i bytes\n", (int)pageSize);
  printf("...adjusting size with page boundaries\n");
  // 8192 - 0 + 4096
  size_t adjustedSize = 0;

  if (size % pageSize == 0)
  {
    adjustedSize = size;
  }
  else
  {
    adjustedSize = (size - (size % pageSize) + pageSize);
  }
  printf("...adjusted size is %i bytes\n", (int)adjustedSize);
  printf("...mapping arena with mmap()\n");

  int fd = open("/dev/zero", O_RDWR);
  _arena_start = mmap(NULL, adjustedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  printf("...arena starts at %p\n", _arena_start);
  //x0900       + 4096 (0x1000) = xa000
  _arena_size += adjustedSize;
  printf("...arena ends at %p\n", _arena_start + _arena_size);

  printf("...initializing header for initial free chunk\n");
  // _arena_start = (struct __node_t *){adjustedSize - sizeof(struct __node_t), 1, NULL, NULL}; // walloc's job // backwards?

  // Place node at start
  struct __node_t *node = _arena_start;
  node->size = adjustedSize - sizeof(struct __node_t);
  node->is_free = 1;
  node->fwd = NULL;
  node->bwd = NULL;

  printf("...header size is %i bytes\n", (int)sizeof(struct __node_t));

  return (int)adjustedSize;
}

int destroy()
{
  statusno = 0;

  if (_arena_start == 0)
  {
    statusno = ERR_UNINITIALIZED;
    return statusno;
  }

  printf("Destroying Arena:\n");
  if (munmap(_arena_start, _arena_size) == -1)
  {
    // ERR_SYSCALL_FAILED
    printf("...error: cannot destroy uninitialized arena. Setting error status\n");
    statusno = ERR_SYSCALL_FAILED;
    return statusno;
  }
  // Success
  printf("...unmapping arena with munmap()\n");
  _arena_start = 0;
  _arena_size = 0;
  return statusno;
}

void *walloc(size_t size)
{
  statusno = 0;
  // IF uninitialized
  if (!_arena_start)
  {
    printf("Error: Unitialized. Setting status code\n");
    statusno = ERR_UNINITIALIZED;
    return NULL; // returns address of status no?
  }

  printf("Allocating memory:\n");
  printf("...looking for free chunk of >= %d bytes\n", (int)size);
  struct __node_t *mem = _arena_start;
  while (mem != NULL)
  {
    // first node free and big enough
    if (mem->is_free == 1 && (mem->size >= size))
    {
      printf("...found free chunk of %d bytes with header at %p\n", (int)mem->size, mem);
      mem->is_free = 0;
      mem->size = size;
      printf("...free chunk->fwd currently points to %p\n", mem->fwd);
      printf("...free chunk->bwd currently points to %p\n", mem->bwd);
      printf("...checking if splitting is required\n");
      /* 
       * Below line: getting new size to pass into the fwd node stores
       * how many bytes of memory can be stored after current mem
      */
      size_t newSize = (_arena_size - ((size_t)mem - (size_t)_arena_start)) - (size + (2 * sizeof(struct __node_t)));
      if (mem->fwd == NULL && ((int)newSize >= 1)) // last node
      {

        struct __node_t *fwd = (struct __node_t *)((char *)mem + size + sizeof(struct __node_t));

        fwd->size = newSize;
        printf("Set new size\n");
        fwd->is_free = 1;
        printf("Set new is free\n");
        fwd->fwd = NULL;
        printf("Set new fwd\n");
        fwd->bwd = mem;
        printf("Set new bwd\n");

        mem->fwd = fwd;
      }
      /* 
       * Else not enough space & last node, set curr size 
       * to fill the rest of the arena ;)
      */
      else if ((int)newSize < 1 && mem->fwd == NULL)
      {
        mem->size = (mem->size + sizeof(struct __node_t) + newSize);
      }
      printf("...splitting not required\n");
      printf("...updating chunk header at %p\n", mem);
      printf("...being careful with my pointer arithmetic and void pointer casting\n");
      printf("...allocation starts at %p\n", mem + 1);
      return mem + 1;
    }
    else
    {
      mem = mem->fwd;
    }
  }

  printf("...no such free chunk exists\n...setting error code\n");
  statusno = ERR_OUT_OF_MEMORY;
  return NULL;
}

void wfree(void *ptr)
{
  printf("Freeing allocated memory:\n");
  printf("...supplied pointer %p\n", ptr);
  printf("...being careful with my pointer arithmetic and void pointer casting\n");

  void *buff = ptr;
  node_t *header = (node_t *)(buff - sizeof(node_t));

  printf("...accessing chunk header at %p\n", header);
  printf("...chunk of size %d\n", (int)header->size);

  header->is_free = 1; // set header to free

  printf("...checking if coalescing is needed\n");
  if (header->fwd != NULL && header->bwd != NULL) // implement coalsecing here
  {
  }
  else
    printf("...coalescing not needed.\n");
  return;
}