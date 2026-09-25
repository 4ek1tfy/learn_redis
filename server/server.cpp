#include "protocol.h"

#include <netinet/in.h>

#define MAXEVENTS 64

static int fd_set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1) return -1;

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int create_server_socket(int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;

    if(fd == -1){
        die("socket()");
    }

    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        die("setsocket()");
    }

    if(fd_set_nonblock(fd) == -1){
        die("fd_set_nonblock()");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(bind(fd, (const struct sockaddr*) &addr, sizeof(addr)) == -1){
        die("bind()");
    }

    if(listen(fd, SOMAXCONN) == -1){
        die("listen()");
    }

    return fd;
}

static void start_server(int port){
    int fd = create_server_socket(port);

    int epoll_fd = epoll_create1(0);

    if(epoll_fd == -1){
        die("epoll_create1()");
    }

    struct epoll_event incoming_events[MAXEVENTS];
    
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);

    while(1){
        int num_fd = epoll_wait(epoll_fd, incoming_events, MAXEVENTS, -1);

        if(num_fd == -1){
            die("epoll_wait()");
        }

        for(int i = 0; i < num_fd; i++){
            if(incoming_events[i].data.fd == fd){
                while(1){
                    errno = 0;
                    int client_fd = accept(fd, NULL, NULL);

                    if(client_fd < 0){
                        if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                        continue;
                    }

                    if(fd_set_nonblock(client_fd) == -1){
                        close(client_fd);
                        continue;
                    }
                    struct epoll_event ev_client;

                    ev_client.events = EPOLLIN | EPOLLET;
                    ev_client.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev_client);
                }
            } else {
                if(incoming_events[i].events & EPOLLIN){
                    one_request(incoming_events[i].data.fd);
                } else{
                    close(incoming_events[i].data.fd);
                }
            }
        }
    }
}

int main(){
    int port = 6767;
    start_server(port);
}