#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "socket_cache.h"

#ifdef DEBUG_MODE
#define DEBUG(f_, ...) printf((f_), ##__VA_ARGS__)
#else
#define DEBUG(f_, ...)
#endif

typedef struct interfaces_s {
    int server_sockets[8];
    uint8_t server_sockets_num;

    int client_sockets[8];
    pthread_t client_thread; // TODO: add 8 thread support
    uint8_t client_sockets_num;

    uint16_t port_server;
    uint16_t port_client;
    uint16_t port;
    bool disabled;

} interfaces_t;

#define MAX_CONNECTIONS 16

uint8_t active_connections = 0;
interfaces_t connections[MAX_CONNECTIONS] = {0};
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //TODO: add 8 mutex support
pthread_cond_t server_ready = PTHREAD_COND_INITIALIZER;
bool server_ready_flag = false;
uint8_t active_con_num = 0;

/* TODO: make this code thread safe */

static bool check_server_socket(interfaces_t* connection, int socket){
    DEBUG("%s: %d\n", __func__, socket);
    /* check if already exists */
    for(uint8_t i = 0; i < connection->server_sockets_num; i++){
        if(connection->server_sockets[i] == socket){
            return true; /* server socket exists */
        }
    }

    return false;
}

static bool check_client_socket(interfaces_t* connection, int socket){
    DEBUG("%s: %d\n", __func__, socket);
    /* check if already exists */
    for(uint8_t i = 0; i < connection->client_sockets_num; i++){
        if(connection->client_sockets[i] == socket){
            return true; /* server socket exists */
        }
    }
    return false;
}


static void add_server_socket(interfaces_t* connection, int socket){
    DEBUG("%s: %d\n", __func__, socket);

    if(!check_server_socket(connection, socket)){
        assert(connection->server_sockets_num < 8);
        connection->server_sockets[connection->server_sockets_num] = socket;
        connection->server_sockets_num++;
    }
}

static void add_client_socket(interfaces_t* connection, int socket){
    DEBUG("%s: %d\n", __func__, socket);

    if(!check_server_socket(connection, socket)){
        assert(connection->client_sockets_num < 8);
        connection->client_sockets[connection->client_sockets_num] = socket;
        connection->client_sockets_num++;
    }
}

static int get_client_socket(interfaces_t* connection){
    DEBUG("%s \n", __func__);

    return connection->client_sockets[connection->client_sockets_num-1];
}

static bool close_server_socket(interfaces_t* connection, int socket){
    DEBUG("%s: %d\n", __func__, socket);

    bool closed = false;

    if(check_server_socket(connection, socket)){

        for(uint8_t i = 0; i < connection->server_sockets_num; i++){

            if(closed){
                connection->server_sockets[i-1] = connection->server_sockets[i];
            }

            if(connection->server_sockets[i] == socket){
                printf("%s -> %d\n", __func__, socket);
                closed = true;
            }

            //connection->client_sockets[connection->client_sockets_num] = socket;
        }
        if(closed){
            connection->server_sockets_num--;
            connection->client_sockets_num--;
        }

    }
    return closed;
}





/* ok */
bool connection_exists(uint16_t port){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d\n", __func__, port);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }
        if(connections[i].port == port){
            pthread_mutex_unlock(&lock);
            return true;
        }
    }

    pthread_mutex_unlock(&lock);
    return false;
}

/* modify -> multi FDs */
bool server_socket_exists(int socket){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d\n", __func__, socket);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }

        if(check_server_socket(&connections[i], socket)){
            pthread_mutex_unlock(&lock);
            return true;
        }

    }

    pthread_mutex_unlock(&lock);
    return false;
}

int server_socket_to_port(int socket){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d\n", __func__, socket);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }

        if(check_server_socket(&connections[i], socket)){
            pthread_mutex_unlock(&lock);
            return connections[i].port;
        }
    }

    pthread_mutex_unlock(&lock);
    return -1;
}


/* unused ? */
bool client_socket_exists(int socket){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d\n", __func__, socket);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }

        if(check_client_socket(&connections[i], socket)){
            pthread_mutex_unlock(&lock);
            return true;
        }

    }

    pthread_mutex_unlock(&lock);
    return false;
}

/* modify -> multi FDs */
bool set_server_socket_to_connection(uint16_t port, int socket){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d %d\n", __func__, socket, port);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }
        if(connections[i].port == port){

            add_server_socket(&connections[i], socket);

            pthread_mutex_unlock(&lock);
            return true;
        }
    }

    pthread_mutex_unlock(&lock);
    return false;
}

/* modify -> multi FDs or keep it? */
bool set_client_socket_to_connection(uint16_t port, int socket){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d %d\n", __func__, socket, port);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }
        if(connections[i].port == port){
            add_client_socket(&connections[i], socket);

            pthread_mutex_unlock(&lock);
            return true;
        }
    }
    pthread_mutex_unlock(&lock);
    return false;
}

int get_client_socket_from_connection(uint16_t port){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d\n", __func__, port);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }
        if(connections[i].port == port){
            int s = get_client_socket(&connections[i]);
            pthread_mutex_unlock(&lock);
            return s;
        }
    }
    pthread_mutex_unlock(&lock);
    return -1;
}

static pthread_t get_thread_id_from_connection(uint16_t port){
    pthread_mutex_lock(&lock);
    pthread_t thread_id;

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }
        if(connections[i].port == port){
            thread_id = connections[i].client_thread;
            pthread_mutex_unlock(&lock);
            return thread_id;
        }
    }
    pthread_mutex_unlock(&lock);
    return -1;
}

void *client_thread_func(void *addr)
{
    DEBUG("%s:\n", "kek");
    struct sockaddr_in *server = (struct sockaddr_in *)addr;
    int socket_desc;

    //Create socket
    DEBUG("%s: create socket...\n", __func__);

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        DEBUG("Could not create socket");
        exit(-1);
    }

    //Connect to remote server
    DEBUG("%s: connect to socket...\n", __func__);
    DEBUG("%s: server port is %d\n", __func__, ntohs(server->sin_port));
    if (connect(socket_desc, (struct sockaddr *)server, sizeof(*server)) < 0)
    {
        DEBUG("connect error %s\n", strerror(errno));
        exit(-1);
    }

    assert(set_client_socket_to_connection(ntohs(server->sin_port), socket_desc));

    DEBUG("Connected. Handshake is done\n");

    /* printf("Locking and waiting\n"); */
    pthread_mutex_lock(&lock);
    while (!server_ready_flag)
        pthread_cond_wait(&server_ready, &lock);
    /* printf("Sending\n"); */

    /* printf("here\n"); */
    char str[14] = "Hello, world!\n";
    write(socket_desc, str, 14);

    server_ready_flag = false;
    pthread_mutex_unlock(&lock);

    free(addr);
    return NULL;
}

void send_malformed_data()
{
    pthread_mutex_lock(&lock);
    server_ready_flag = true;
    pthread_mutex_unlock(&lock);

    pthread_cond_signal(&server_ready);
}

void create_client(const char *ip, uint16_t port)
{
    printf("%s: %s %d\n", __func__, ip, port);

    struct sockaddr_in *server = malloc(sizeof(struct sockaddr_in));
    server->sin_addr.s_addr = inet_addr(ip);
    server->sin_family = AF_INET;
    server->sin_port = htons(port);

    pthread_t thread_id = get_thread_id_from_connection(port);

    pthread_create(&thread_id, NULL, client_thread_func, server);
}

/* ok */
bool add_connection(uint16_t port){
    printf("%s: %d\n", __func__, port);

    assert(!connection_exists(port));

    pthread_mutex_lock(&lock);

    if(active_con_num >= MAX_CONNECTIONS){
        /* TODO: replace with ABORT hypercall */
        printf("%s -> release\n", __func__);
        pthread_mutex_unlock(&lock);
        abort();
    }
    printf("%s -> port: %d\n", __func__, port);
    connections[active_connections].port = port;

    /* init fd cache */
    connections[active_connections].client_sockets_num = 0;
    connections[active_connections].server_sockets_num = 0;

    active_connections++;
    active_con_num++;
    pthread_mutex_unlock(&lock);
    return true;
}

/* modify -> multi FDs */
int set_select_fds(fd_set *set, fd_set *old_set){
    pthread_mutex_lock(&lock);
    printf("%s -> %d\n", __func__, active_connections);
    int ret = 0;

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }

        for(uint8_t j = 0; j < connections[i].server_sockets_num; j++){
            if(FD_ISSET(connections[i].server_sockets[j], old_set)){
                FD_SET(connections[i].server_sockets[j], set);
                ret++;
            }
        }
    }

    return ret;

}

/* modify -> multi FDs */
void disable_connection_by_server_socket(int socket){
    pthread_mutex_lock(&lock);
    DEBUG("%s: %d\n", __func__, socket);

    for(uint8_t i = 0; i < active_connections; i++){
        if(connections[i].disabled == true){
            continue;
        }

        if(close_server_socket(&connections[i], socket) && connections[i].server_sockets_num == 0){
            active_con_num--;
            connections[i].disabled = true;
            //TODO pthread_join here??
            pthread_mutex_unlock(&lock);
            return;
        }

    }
    pthread_mutex_unlock(&lock);
}

uint16_t get_active_connections(void){
    DEBUG("%s\n", __func__);

    return active_con_num;
}
