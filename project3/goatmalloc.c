#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "goatmalloc.h"

static void *_arena_start;
size_t _arena_size;
int statusno;

int init(size_t size)
{
  printf("Initializing arena:\n");

  _arena_start = 0;
  statusno = 0;
  size_t adjustedSize = 0;
  size_t pageSize = getpagesize();

  printf("...requested size %.00f bytes\n", (double)size); // casting to double to convert -1 to expected output

  /* If size passed in is a bad argument */
  if ((double)size > MAX_ARENA_SIZE) // casting to double to convert -1 to expected output
  {
    printf("...error: requested size larger than MAX_ARENA_SIZE (%d)\n", MAX_ARENA_SIZE);
    statusno = ERR_BAD_ARGUMENTS;
    return statusno;
  }

  printf("...pagesize is %i bytes\n", (int)pageSize);

  /* If the size is not a multiple of page size, adjust it */
  if (size % pageSize == 0)
  {
    adjustedSize = size;
  }
  else
  {
    printf("...adjusting size with page boundaries\n");
    adjustedSize = size - (size % pageSize) + pageSize;
    printf("...adjusted size is %i bytes\n", (int)adjustedSize);
  }

  printf("...mapping arena with mmap()\n");
  int fd = open("/dev/zero", O_RDWR);
  _arena_start = mmap(NULL, adjustedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  printf("...arena starts at %p\n", _arena_start);

  _arena_size += adjustedSize; // adjust arena size with adjusted size
  printf("...arena ends at %p\n", _arena_start + _arena_size);

  printf("...initializing header for initial free chunk\n");

  /* initializing header below */
  node_t *header = _arena_start;
  header->size = adjustedSize - sizeof(node_t);
  header->is_free = 1;
  header->fwd = NULL;
  header->bwd = NULL;
  printf("...header size is %i bytes\n", (int)sizeof(node_t));

  return (int)adjustedSize;
}

int destroy()
{
  statusno = 0;

  printf("Destroying Arena:\n"); // nooooooo D:

  if (_arena_start == 0) // arena is uninitialized, cannot destroy it
  {
    printf("...error: cannot destroy uninitialized arena. Setting error status\n");
    statusno = ERR_UNINITIALIZED;
    return statusno;
  }

  printf("...unmapping arena with munmap()\n");
  /* munmap system call, if it fails we set status to syscall failed */
  if (munmap(_arena_start, _arena_size) == -1)
  {
    printf("...error: system call failed");
    statusno = ERR_SYSCALL_FAILED;
    return statusno;
  }

  _arena_start = 0;
  _arena_size = 0;
  return statusno;
}

void *walloc(size_t size)
{
  statusno = 0;

  if (!_arena_start) // can't walloc to an uninitialized arena
  {
    printf("Error: Unitialized. Setting status code\n");
    statusno = ERR_UNINITIALIZED;
    return NULL;
  }

  printf("Allocating memory:\n");
  printf("...looking for free chunk of >= %d bytes\n", (int)size);

  node_t *mem = _arena_start; // header at start
  while (mem != NULL)
  {
    if (mem->is_free == 1 && (mem->size >= size)) // first node free and big enough
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
      size_t newSize = (_arena_size - ((size_t)mem - (size_t)_arena_start)) - (size + (2 * sizeof(node_t)));
      if (mem->fwd == NULL && ((int)newSize >= 1)) // last node
      {
        node_t *fwd = (node_t *)((char *)mem + size + sizeof(node_t));
        fwd->size = newSize;
        fwd->is_free = 1;
        fwd->fwd = NULL;
        fwd->bwd = mem;
        mem->fwd = fwd;
      }
      /* 
	   * else not enough space & last node, set curr size 
	   * to fill the rest of the arena ;)
	   */
      else if ((int)newSize < 1 && mem->fwd == NULL)
      {
        mem->size = (mem->size + sizeof(node_t) + newSize);
      }
      printf("...splitting not required\n");
      printf("...updating chunk header at %p\n", mem);
      printf("...being careful with my pointer arithmetic and void pointer casting\n");
      printf("...allocation starts at %p\n", mem + 1);
      return mem + 1; // pointer arithmetic uwu (mem + 1(sizeof mem))
    }
    else
      mem = mem->fwd;
  }

  printf("...no such free chunk exists\n...setting error code\n");
  statusno = ERR_OUT_OF_MEMORY;
  return NULL;
}

void wfree(void *ptr)
{
  int frees = 0; // keeping track of which coalescing case
  printf("Freeing allocated memory:\n");
  printf("...supplied pointer %p:\n", ptr);
  printf("...being careful with my pointer arithmetic and void pointer casting\n");

  void *buff = ptr;
  node_t *header = (node_t *)(buff - sizeof(node_t));

  printf("...accessing chunk header at %p\n", header);
  printf("...chunk of size %d\n", (int)header->size);

  header->is_free = 1; // set header to free

  printf("...checking if coalescing is needed\n");

  /* forward node free case */
  if (header->fwd != NULL && header->fwd->is_free == 1)
  {
    frees = 3;
    header->size = header->size + header->fwd->size + sizeof(node_t);
    header->fwd = header->fwd->fwd;
    if (header->fwd != NULL)
    {
      header->fwd->fwd->bwd = header;
    }
  }

  /* backward node free case */
  if (header->bwd != NULL && header->bwd->is_free == 1)
  {
    frees = (frees == 3) ? 1 : 2;
    header->bwd->size = header->size + header->bwd->size + sizeof(node_t);
    header->bwd->fwd = header->fwd;
    if (header->fwd != NULL)
    {
      header->fwd->bwd = header->bwd;
    }
  }

  switch (frees)
  {
  case 1:
    printf("...col. case 1: previous, current, and next chunks all free.\n");
    break;
  case 2:
    printf("...col. case 2: previous and current chunks free.\n");
    break;
  case 3:
    printf("...col. case 3: current and next chunks free.\n");
    break;
  default:
    printf("...coalescing not needed.\n");
    break;
  }

  return;
}
