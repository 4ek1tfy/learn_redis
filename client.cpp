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

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd < 0){
        die("socket()");
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6767);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int rv = connect(fd, (const struct sockaddr *) &addr, sizeof(addr));

    if(rv < 0){
        die("connect()");
    }

    char message[] = "Hello!";
    write(fd, message, strlen(message));

    char read_buff[64];
    ssize_t n = read(fd, read_buff, sizeof(read_buff) - 1);

    if(n < 0){
        die("read()");
    }

    read_buff[n] = '\0'; 

    printf("server says: %s\n", read_buff); 
    
    close(fd);
}