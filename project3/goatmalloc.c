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

// use OS commands (start and ending bytes from OS)
// after make sure it's a multiple of the page size
// allocate space in memory of type __node_t with size adjusted size

int init(size_t size)
{
    _arena_start = 0;
    statusno = 0;

    if ((int)size < 1)
    {
        // Invalid size
        statusno = ERR_BAD_ARGUMENTS;
        return statusno;
    }

    printf("Initializing arena:\n");
    size_t pageSize = getpagesize();

    printf("...requested size %i bytes\n", (int)size);
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

    // MOVE THIS INTO WALLOC
    struct __node_t *node = _arena_start;
    node->size = adjustedSize; // - sizeof(struct __node_t);
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
    printf("...unmapping arena with munmap()\n");
    if (munmap(_arena_start, _arena_size) == -1)
    {
        // ERR_SYSCALL_FAILED
        statusno = ERR_SYSCALL_FAILED;
        return statusno;
    }
    // Success
    _arena_start = 0;
    _arena_size = 0;
    return statusno;
}

void *walloc(size_t size)
{
    statusno = 0;
    printf("size passed in: %d\n", (int)size);

    // IF uninitialized
    if (!_arena_start)
    {
        printf("mem uninitialized\n");
        statusno = ERR_UNINITIALIZED;
        return NULL; // returns address of status no?
    }

    struct __node_t *mem = _arena_start;
    printf("MEM SIZE = %d\n", (int)mem->size);
    while (mem != NULL)
    {
        printf("MEM ADDRESS %p\n", mem);
        // first node free and big enough
        if (mem->is_free == 1 && (mem->size - sizeof(struct __node_t)) >= size)
        {
            printf("mem size - sizeof struct = %d\n", (int)(mem->size - sizeof(struct __node_t)));
            mem->is_free = 0;
            mem->size = (size + sizeof(struct __node_t));
            printf("NEW MEM SIZE = %d\n", (int)mem->size);
            printf("mem found\n");
            return mem;
        }
        else
            break;
        mem = mem->fwd;
    }

    printf("mem not found :(\n");
    statusno = ERR_OUT_OF_MEMORY;
    return NULL;
}