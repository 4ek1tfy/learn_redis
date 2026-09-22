#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void die(const char* message){
    perror(message);
    exit(EXIT_FAILURE); 
}

void msg(const char* message){
    perror(message);
}

static void do_something(int connfd){
    char read_buff[64];
    ssize_t n = read(connfd, read_buff, sizeof(read_buff) - 1);

    if(n < 0){
        msg("read() - error\n");
        return;
    }

    read_buff[n] = '\0';

    printf("client says: %s\n", read_buff);

    char write_buff[] = "world"; 
    write(connfd, write_buff, strlen(write_buff));
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0){
        die("socket()");
    }
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6767);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = bind(fd, (const struct sockaddr *)& addr, sizeof(addr));
    if(rv){
        die("bind");
    }

    rv = listen(fd, SOMAXCONN);

    if(rv < 0){
        die("listen()");
    }

    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t addr_len = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *) &client_addr, &addr_len);

        if(connfd < 0){
            continue;
        }

        do_something(connfd);
        close(connfd);
    }

    close(fd);
}