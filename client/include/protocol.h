#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

static inline void die(const char* message){
    perror(message);
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

static inline int32_t write_full(int fd, const char* buff, size_t n){
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

static inline int32_t query(int fd, const char* text){
    uint32_t len = static_cast<uint32_t>(strlen(text));
    uint32_t net_len = htonl(len);

    if(len > k_max_msg){
        return -1;
    }

    char wbuff[4 + k_max_msg];

    memcpy(wbuff, &net_len, 4);
    memcpy(&wbuff[4], text, len);

    int32_t err = write_full(fd, wbuff, 4 + len);
    if(err){
        return err;
    }

    char rbuff[4 + k_max_msg];

    errno = 0;
    err = read_full(fd, rbuff, 4);

    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    memcpy(&net_len, rbuff, 4);
    len = ntohl(net_len);
    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    err = read_full(fd, &rbuff[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    printf("server says: %.*s\n", len, &rbuff[4]);
    return 0;
}