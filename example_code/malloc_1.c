#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>

//this segfaults, why?
void *malloc(size_t size) //same signature as the real malloc function
{
    fprintf(stderr, "[1] Hooked libc function call: malloc(%ld)\n", size);
    return 0;
}
