#ifndef MYAPI_H
#define MYAPI_H
#include <netinet/in.h>
#include <pthread.h>
#include	<signal.h>
#include	<errno.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include <stdio.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <ifaddrs.h>
#include	<strings.h>
#include        <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<semaphore.h>
#define REQ 1
#define REP 2
#define HELLO 3
#define HELLO_ACK 4


typedef struct Msg {
    int balance;
    int	socket_fd, fsize;
    struct sockaddr_in	s_in, from;
    char ip_adr[16];
    sem_t sema;
    pid_t pid;
    int status;
    char  defered_array[17][17];
    int def_count;
    pthread_t thread_id;
    struct hostent *hostptr;
    int critical;
    int host_count;
    int requesting;
    u_int clock;
} Msg;

typedef struct packet{
    u_short req_type;
    u_int clock;
}packet;



void insert_ip(Msg *msg);
void init(Msg *msg);
void lock(Msg *msg);
void unlock(Msg *msg);


#endif 
