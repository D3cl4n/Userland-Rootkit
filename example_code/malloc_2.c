#define _GNU_SOURCE

#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


//function pointer to the original malloc, initialized to NULL
static void* (*original_malloc)(size_t) = NULL;

void *malloc(size_t size)
{
    if (original_malloc == NULL)
    {
        //finds the address of the next occurence of "malloc"
        original_malloc = (void*(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    }

    fprintf(stderr, "[2] Hooked libc function call: malloc(%ld)\n", size);

    //perform the functionality of the real malloc function
    void *allocation = NULL;
    allocation = original_malloc(size);

    return allocation;
}