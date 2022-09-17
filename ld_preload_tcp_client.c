#include <stdint.h>
#include <sys/poll.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdbool.h>
#include <assert.h>
#include <poll.h>
#include <unistd.h>

#include "socket_cache.h"
#include "syscalls.h"

#include <errno.h>

#include <time.h>
#include <sys/epoll.h>

/* TODO: eliminate sleep & usleep und bla */

//#define TARGET_PORT 5353
//#define TARGET_PORT 5158
//#define TARGET_PORT 8000

//#define DEBUG_MODE

#define UNUSED(x) (void)(x)
#define DEBUG(f_, ...) printf((f_), ##__VA_ARGS__)

__attribute__ ((noreturn)) static void not_implemented(const char* func) {
    printf("NOT IMPLEMENTED <%s>\n", func);
    abort();
}

/* unsigned int sleep(unsigned int seconds){ */
/*     DEBUG("%s: %u\n", __func__, seconds); */
/*     // nope */
/*     return 0; */
/* } */


/* int usleep(useconds_t usec){ */
/*     DEBUG("%s: %u\n", __func__, usec); */
/*     /1* nope *1/ */
/*     return 0; */
/* } */

int shutdown(int sockfd, int how){
    printf("shutdown %d\n", how);
    return close(sockfd);
    //not_implemented(__func__);
}

int ioctl(int fd, int cmd, void *argp){
    if(server_socket_exists(fd)){
        return 0;
    }
    return real_ioctl(fd, cmd, argp);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt){
    DEBUG("%s --> %d\n", __func__, fd);
    if(server_socket_exists(fd)){
        //int ret = handle_next_packet_iovec(fd, iov, iovcnt, false);
        int ret = 0;
        printf("%s -> %d\n", __func__, ret);
        return ret;
    }
    return real_readv(fd, iov, iovcnt);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt){
    DEBUG("%s --> %d\n", __func__, fd);
    if(server_socket_exists(fd)){
        ssize_t len = 0;
        for(int i = 0; i < iovcnt; i++){
            len += iov[i].iov_len;
        }
        return len;
    }
    return real_writev(fd, iov, iovcnt);
}

ssize_t preadv(int fd, const struct iovec *iov, int iovcnt,
               off_t offset){
    DEBUG("%s --> %d\n", __func__, fd);
    if(server_socket_exists(fd)){
        not_implemented(__func__);
    }
    return real_preadv(fd, iov, iovcnt, offset);
}

ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt,
                off_t offset){
    DEBUG("%s --> %d\n", __func__, fd);
    if(server_socket_exists(fd)){
        not_implemented(__func__);
    }
    return real_pwritev(fd, iov, iovcnt, offset);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags){

    DEBUG("%s --> %d\n", __func__, sockfd);
    if(server_socket_exists(sockfd)){
        DEBUG("%s: sockfd: %d\n", __func__, sockfd);
        return len;
    }

    return real_send(sockfd, buf, len, flags);
}

ssize_t write(int fd, const void *buf, size_t count){
    /* DEBUG("%s --> %d %zu\n", __func__, fd, count); */
    /* DEBUG("%s: %p %ld\n", __func__, buf, count); */
    //if(server_socket_exists(fd)){
    //    return count;
    //}

    return real_write(fd, buf, count);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen){
    DEBUG("%s --> %d\n", __func__, sockfd);

    DEBUG("%s: sockfd: %d (%p %lx)\n", __func__, sockfd, buf, len);
    printf("%s returning: %zu\n", __func__, len);
    return real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t read(int fd, void *buf, size_t count){
    printf("==== %s ==== %zu\n", __func__, count);
    DEBUG("%s --> %d\n", __func__, fd);

    if(server_socket_exists(fd)){
        //notify client (thread)
        /* sleep(2); */
        send_malformed_data();
        return real_read(fd, buf, count);
        //init_nyx();
        //TODO return handle_next_packet(fd, buf, count, false);
        //printf("%s: not implemented\n", __func__);
        //exit(0);
    }
    return real_read(fd, buf, count);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags){
    int ret = 0;
    DEBUG("%s --> %d\n", __func__, sockfd);

    if(server_socket_exists(sockfd)){

    }
    ret = real_recv(sockfd, buf, len, flags);
    printf("%s: %d\n", __func__, ret);
    return ret;
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    DEBUG("%s --> %d\n", __func__, sockfd);

    if(server_socket_exists(sockfd)){

        //init_nyx();



        //size_t ret =  handle_next_packet(sockfd, buf, len, flags & MSG_TRUNC);
        size_t ret = 1;

        /* in case of UDP -> emulate src_addr (**I'm going to hell for this**) */
        /* TODO: check if socket type is DGRAM */
        if(src_addr != NULL){
            struct sockaddr_in* tmp = (struct sockaddr_in*) src_addr;
            tmp->sin_family = AF_INET;
#ifdef CLIENT_UDP_PORT
            tmp->sin_port = htons(CLIENT_UDP_PORT);
#else
            tmp->sin_port = htons(50000);
            tmp->sin_addr.s_addr = htonl(0x7F000001); /* 127.0.0.1 */
            *addrlen = sizeof(struct sockaddr_in);
        }

        printf("%s returning: %zu\n", __func__, ret);
        return ret;
#endif
    }
    ssize_t ret = real_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    DEBUG("%s ret=%zd\n", __func__, ret);
    /*
        if (ret < 0){

        DEBUG("%s error: %s", __func__, strerror(errno));
        while(1){

        }
        }
        */
    return ret;
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags){

    DEBUG("%s --> %d\n", __func__, sockfd);

    if(server_socket_exists(sockfd)){

        printf("%s: not implemented\n", __func__);
        exit(0);
    }
    return real_recvmsg(sockfd, msg, flags);
}

int dup(int oldfd){
    if(server_socket_exists(oldfd)){

        DEBUG("DUP callend!\n");

        int ret = real_dup(oldfd);

        if(ret != -1){
            /* close newfd first if it is already registered in our socket cache */
            //disable_connection_by_server_socket(ret);
            /* bind newfd to our port */
            int port = server_socket_to_port(oldfd);

            if(port == -1){
                printf("FAIL in %s!", __func__);
            }
            assert(port != -1);
            set_server_socket_to_connection(port, ret);

            DEBUG("DUP -> oldfd: %d is now mapped to fd: %d!\n", oldfd, ret);
        }

        return ret;
    }
    return real_dup(oldfd);

    /*

        if(server_socket_exists(oldfd)){
        DEBUG("DUP!\n");
        printf("DUP!\n");
        while(1){}
        }

        return real_dup(oldfd);
        */
}

int dup2(int oldfd, int newfd){
    if(server_socket_exists(oldfd)){

        //close(newfd);


        DEBUG("DUP2 callend!\n");

        int ret = real_dup2(oldfd, newfd);

        if(ret != -1){
            /* close newfd first if it is already registered in our socket cache */
            disable_connection_by_server_socket(ret);
            /* bind newfd to our port */
            int port = server_socket_to_port(oldfd);

            if(port == -1){
                printf("FAIL in %s!", __func__);
            }
            assert(port != -1);
            set_server_socket_to_connection(port, ret);

            DEBUG("DUP2 -> oldfd: %d is now mapped to newfd: %d!\n", oldfd, ret);
        }

        return ret;


        DEBUG("DUP2!\n");
        printf("DUP2!\n");

        while(1){}
    }

    return real_dup2(oldfd, newfd);
}

int dup3(int oldfd, int newfd, int flags){
    if(server_socket_exists(oldfd)){
        DEBUG("DUP3!\n");
        printf("DUP3!\n");

        while(1){}
    }

    return real_dup3(oldfd, newfd, flags);
}

int close(int fd){
    DEBUG("%s: %d\n", __func__, fd);

    if(server_socket_exists(fd)){
        DEBUG("%s: %d\n", __func__, fd);

        disable_connection_by_server_socket(fd);
        DEBUG("%s: %d => %d\n", __func__, fd, get_active_connections() );

        /* broken ? */
        if(get_active_connections() == 0){
            DEBUG("RELEASE!\n");
            return real_close(fd);
            //while(1){}
            //
            //}
            //printf("=============\n");
            //do_heap_stuff();
        exit(0);
    }

}
return real_close(fd);
}

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout){

    UNUSED(timeout);

    /* disable timeouts */
    struct timeval timeout_new;
    timeout_new.tv_sec = 0;
    timeout_new.tv_usec = 0;

    fd_set old_readfds;

    if(readfds){
        memcpy(&old_readfds, readfds, sizeof(fd_set));
    }

    int ret = real_select(nfds, readfds, writefds, exceptfds, &timeout_new);

    //timeout_new.tv_sec = 4;
    //timeout_new.tv_usec = 100;

    if(get_active_connections() && readfds){
        printf("%s\n", __func__);

        return ret + set_select_fds(readfds, &old_readfds);
        //return 1; /* fix this dude */

        /* todo update ret? */
    }

    return ret;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout){
    DEBUG("%s: %lu\n", __func__, nfds);

    /* todo: more features ? */
    for(nfds_t i = 0; i < nfds; i++){
        fds[i].revents = 0;
        if(((fds[i].events & POLLIN) || (fds[i].events & POLLOUT)) && server_socket_exists(fds[i].fd)){
            fds[i].revents = ((fds[i].events & POLLIN) | (fds[i].events & POLLOUT));
            printf("%s: RETURNING -> %d\n", __func__, fds[i].fd);
            return 1;
        }
    }
    DEBUG("%s: DEFAULT %lu -> timeout: %d\n", __func__, nfds, timeout);

    int ret = real_poll(fds, nfds, 0);
    return ret;
}

int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags){
    printf("============ %s ===========\n", __func__);
    //sleep(1);

    int ret = real_accept4(sockfd, addr, addrlen, flags);

    struct sockaddr_in tmp_addr;
    int len = sizeof(struct sockaddr);
    if(ret != -1 && getsockname(sockfd, (struct sockaddr *) &tmp_addr, (void*)&len) != -1){
        printf("%s: port number %d\n", __func__, ntohs(tmp_addr.sin_port));
        if(ret != -1) {
            printf("%s: %d %d\n", __func__, ntohs(tmp_addr.sin_port), ret);
            assert(set_server_socket_to_connection(ntohs(tmp_addr.sin_port), ret));
        }
        printf("%s: port number %d\n", __func__, ntohs(tmp_addr.sin_port));
    }


    return ret;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    return accept4(sockfd, addr, addrlen, 0);
}




/*
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    printf("============ %s ===========", __func__);
    int ret = real_accept(sockfd, addr, addrlen);

    struct sockaddr_in tmp_addr;
    int len = sizeof(struct sockaddr);
    if(getsockname(sockfd, (struct sockaddr *) &tmp_addr, &len) != -1){

    printf("%s %d %d\n", __func__, tmp_addr.sin_port, ret);

    assert(set_server_socket_to_connection(ntohs(tmp_addr.sin_port), ret));
    DEBUG("%s: port number %d\n", __func__, ntohs(tmp_addr.sin_port));
    }

    return ret;
    }

    int accept4(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    accept(sockfd, addr, addrlen);
    }
    */

int listen(int sockfd, int backlog){
    struct sockaddr_in addr;
    int len = sizeof(struct sockaddr);

    printf("=== %s\n", __func__);

    int ret = -1;

    if(getsockname(sockfd, (struct sockaddr *) &addr, (void*)&len) != -1){

        bool exists = connection_exists(ntohs(addr.sin_port));

        if(!exists){
            printf("%s: port number %d\n", __func__, ntohs(addr.sin_port));
        }

        ret = real_listen(sockfd, backlog);

        if(!exists){
            add_connection(ntohs(addr.sin_port));
            create_client("127.0.0.1", ntohs(addr.sin_port)); //creates thread
            printf("%s: DONE \n", "kek1");
        }
    }

    return ret;
}

/* todo: add support for multiple fds */
uint64_t u64_tmp = 0;
int fd_tmp = 0;
int epfd_tmp = 0;
uint32_t events_tmps = 0;

/* fix this */
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event){
    if(server_socket_exists(fd)){
        assert(op == EPOLL_CTL_ADD || op == EPOLL_CTL_MOD || op == EPOLL_CTL_DEL);
        if(op == EPOLL_CTL_DEL){
            u64_tmp = 0;
            events_tmps = 0;
            fd_tmp = 0;
            epfd_tmp = 0;
        }
        else{
            u64_tmp = event->data.u64;
            events_tmps = event->events;
            fd_tmp = fd;
            epfd_tmp = epfd;
        }
        return 0;
    }
    return real_epoll_ctl(epfd, op, fd, event);
}


int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout){
    UNUSED(timeout);

    int ret = 0;

    if(u64_tmp && epfd == epfd_tmp){
        ret = real_epoll_wait(epfd, events, maxevents, 0);
        events[ret].data.u64 = u64_tmp;
        events[ret].events = events_tmps;
        ret++;
    }
    else{
        ret = real_epoll_wait(epfd, events, maxevents, 0);
    }
    return ret;
}

static void init() __attribute__((constructor));

void init() {
    printf("in init function of %s\n", __FILE__);
    init_syscall_fptr();
}
