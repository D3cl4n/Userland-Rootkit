#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main() 
{
    char *allocation = (char *)malloc(0x100);
    fprintf(stdout, "Malloc returned: %p\n", allocation);
    return 0;
}