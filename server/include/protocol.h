#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <stdexcept>
#include <system_error>

static inline void die(const char* message) {
    int current_errno = errno; 

    if (message && *message) {
        fprintf(stderr, "%s: ", message);
    }

    if (current_errno != 0) {
        fprintf(stderr, "%s\n", strerror(current_errno));
    } else {
        fprintf(stderr, "Unknown/unspecified error\n");
    }

    fflush(stderr);
    fflush(stdout);

    exit(EXIT_FAILURE);
}


static inline void msg(const char* message){
    if (errno == 0) {
        fprintf(stderr, "%s\n", message);
    } else {
        perror(message);
    }
}


static const size_t k_max_msg = 4096;

static inline int32_t read_full(int fd, char* buff, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, buff, n);

        if(rv <= 0){
            return -1;
        }

        assert(static_cast<size_t>(rv) <= n);
        n -= static_cast<size_t>(rv);
        buff += static_cast<size_t>(rv);
    }

    return 0;
}

static inline int32_t write_full(int fd, char* buff, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buff, n);

        if(rv <= 0){
            return -1;
        }

        assert(static_cast<size_t>(rv) <= n);
        n -= static_cast<size_t>(rv);
        buff += static_cast<size_t>(rv);
    }

    return 0;
}

inline int32_t one_request(int connfd){
    char rbuff[4 + k_max_msg];
    errno = 0;

    int32_t err = read_full(connfd, rbuff, 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t net_len = 0;

    memcpy(&net_len, rbuff, 4);

    uint32_t len = ntohl(net_len);

    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    err = read_full(connfd, &rbuff[4], len);

    if(err){
        msg("read() error");
        return err;
    }

    printf("client says: %.*s\n", len, &rbuff[4]);

    return write_full(connfd, rbuff, 4 + len);
}