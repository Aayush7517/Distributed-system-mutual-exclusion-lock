#include "myapi.h"
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <strings.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int thread = 0;

void *listen_to_call(void *msg);
void lock(Msg *msg);
void insert_ip(Msg *msg)
{
    FILE *fp; // declare a file pointer
    char final_ip[16];
    fp = fopen("example.txt", "a"); // open file in append mode

    if (fp == NULL)
    {
        printf("Error: Unable to open file.\n");
        exit(1);
    }

    struct ifaddrs *addrs, *tmp;
    int status;

    // Call getifaddrs() to obtain the list of network interfaces
    status = getifaddrs(&addrs);
    if (status != 0)
    {
        printf("Error: %s\n", strerror(status));
        exit(1);
    }

    // Loop through the list of network interfaces

    for (tmp = addrs; tmp != NULL; tmp = tmp->ifa_next)
    {
        if (tmp->ifa_addr == NULL)
            continue;

        // Check if the interface is an AF_INET interface
        if (tmp->ifa_addr->sa_family == AF_INET)
        {
            // Convert the IP address to a string
            struct sockaddr_in *addr = (struct sockaddr_in *)tmp->ifa_addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);

            // Print the IP address
            strcpy(final_ip, ip);
            strncpy(msg->ip_adr, ip, sizeof(msg->ip_adr) - 1);
            msg->ip_adr[sizeof(msg->ip_adr) - 1] = '\0';
            strcat(final_ip, "\n");
        }
    }

    freeifaddrs(addrs);
    fprintf(fp, "%s", final_ip);
    fclose(fp);
}
int a = 1;
void init(Msg *msg)
{
    msg->critical = 0;
    msg->requesting = 0;
    msg->def_count = 0;
    msg->host_count = 0;
    msg->clock = 0;
    insert_ip(msg);
    pthread_create(&msg->thread_id, NULL, listen_to_call, (void *)msg);
}

void
    printsin(sin, m1, m2) struct sockaddr_in *sin;
char *m1, *m2;
{

    printf("%s %s:\n", m1, m2);
    printf("  family %d, addr %s, port %d\n", sin->sin_family,
           inet_ntoa(sin->sin_addr), ntohs((unsigned short)(sin->sin_port)));
}

void *listen_to_call(void *msg)
{
    Msg *msg1 = (Msg *)msg;
    int value;
    packet pck;
    int reply_count = 0;
    msg1->def_count = 0;
    msg1->critical = 0;
    msg1->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (msg1->socket_fd < 0)
    {
        perror("recv_udp:socket");
        exit(1);
    }

    bzero((char *)&msg1->s_in, sizeof(msg1->s_in));
    msg1->s_in.sin_family = (short)AF_INET;
    msg1->s_in.sin_addr.s_addr = htonl(INADDR_ANY);
    msg1->s_in.sin_port = htons((u_short)0x7778);
    // printsin(&msg1->s_in, "RECV_UDP", "Local socket is:");
    fflush(stdout);
    if (bind(msg1->socket_fd, (struct sockaddr *)&msg1->s_in, sizeof(msg1->s_in)) < 0)
    {
        perror("recv_udp:bind");
        exit(1);
    }
    int cc, count = 0;

    for (;;)
    {
        socklen_t fsize = sizeof(msg1->from);
        cc = recvfrom(msg1->socket_fd, &pck, sizeof(pck), 0, (struct sockaddr *)&msg1->from, &fsize);
        if (cc < 0)
            perror("recv_udp:recvfrom");

        if (pck.req_type == REP)
        {
            count++;
            if (count >= msg1->host_count)
            {
                printf("In critical section\n");
                count = 0;
                msg1->def_count = 0;
                sem_post(&msg1->sema);
            }
        }
        if (pck.req_type == REQ)
        {

            // printf("%s\n", inet_ntoa(msg1->from.sin_addr));
            if (msg1->critical == 1)
            {
                char ip[17];
                strcpy(ip, inet_ntoa(msg1->from.sin_addr));
                // strcpy(msg1->defered_array[msg1->def_count], ip);
                snprintf(msg1->defered_array[msg1->def_count], sizeof(msg1->defered_array[msg1->def_count]), "%s", ip);
                msg1->def_count += 1;
                continue;
            }
            if (msg1->requesting == 0)
            {
                if (pck.clock > msg1->clock)
                {
                    msg1->clock = pck.clock;
                }
                pck.req_type = REP;
                cc = sendto(msg1->socket_fd, &pck, sizeof(pck), 0, (struct sockaddr *)&msg1->from,
                            sizeof(msg1->from));
                if (cc < 0)
                {
                    perror("sendto:");
                }
                continue;
            }

            if (msg1->requesting == 1)
            {
                if (msg1->clock <= pck.clock)
                {
                    char ip[17];
                    strcpy(ip, inet_ntoa(msg1->from.sin_addr));
                    snprintf(msg1->defered_array[msg1->def_count], sizeof(msg1->defered_array[msg1->def_count]), "%s", ip);
                    msg1->def_count += 1;
                    continue;
                }
                if (msg1->clock >= pck.clock)
                {
                    pck.req_type = REP;
                    cc = sendto(msg1->socket_fd, &pck, sizeof(pck), 0, (struct sockaddr *)&msg1->from,
                                sizeof(msg1->from));
                    if (cc < 0)
                    {
                        perror("sendto:");
                    }
                    continue;
                }
            }
        }

        // printsin(&msg1->from, "recv_udp:", "Packet from:");

        //printf("Received value: %d\n", pck.req_type);
        fflush(stdout);
    }
}

void lock(Msg *msg)
{

    packet pck;
    struct hostent *hostptr;
    struct sockaddr_in dest;
    char filename[] = "example.txt";
    char ip_address[17];

    pck.clock = msg->clock;
    msg->requesting = 1;

    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open file %s\n", filename);
        exit(1);
    }
    msg->host_count = 0;
    sem_init(&msg->sema, 0, 0);
    while (fgets(ip_address, 16, fp) != NULL)
    {

        // remove newline character from ip_address string
        ip_address[strcspn(ip_address, "\n")] = 0;

        if (strcmp(msg->ip_adr, ip_address) == 0)
        {
        }
        else
        {
            msg->host_count += 1;
            bzero((char *)&dest, sizeof(dest)); /* They say you must do this */
            if ((hostptr = gethostbyname(ip_address)) == NULL)
            {
                fprintf(stderr, "send_udp: invalid host name, %s\n", ip_address);
                exit(1);
            }
            int cc;
            pck.req_type = REQ;
            dest.sin_family = AF_INET;
            bcopy(hostptr->h_addr, (char *)&dest.sin_addr, hostptr->h_length);
            dest.sin_port = htons((u_short)0x7778);

            cc = sendto(msg->socket_fd, &pck, sizeof(pck), 0, (struct sockaddr *)&dest,
                        sizeof(dest));
            if (cc < 0)
            {
                perror("send_udp:sendto");
                exit(1);
            }
        }
    }
    fclose(fp);
    sem_wait(&msg->sema);
    msg->requesting = 0;
    msg->critical = 1;
    msg->clock += 1;
}

void unlock(Msg *msg)
{
    msg->critical = 0; // put req in defered array
    struct sockaddr_in dest;
    struct hostent *hostptr;
    packet pck;
    pck.req_type = REP;
    msg->clock +=1;
    //printf("deffered count %d\n", msg->def_count);
    while (msg->def_count > 0)
    {
        bzero((char *)&dest, sizeof(dest)); /* They say you must do this */
        if ((hostptr = gethostbyname(msg->defered_array[msg->def_count - 1])) == NULL)
        {
            fprintf(stderr, "send_udp: invalid host name, %s\n", msg->defered_array[msg->def_count]);
            exit(1);
        }

        dest.sin_family = AF_INET;
        bcopy(hostptr->h_addr, (char *)&dest.sin_addr, hostptr->h_length);
        dest.sin_port = htons((u_short)0x7778);
        int cc;
        pck.req_type = REP;
        cc = sendto(msg->socket_fd, &pck, sizeof(pck), 0, (struct sockaddr *)&dest,
                    sizeof(dest));
        if (cc < 0)
        {
            perror("send_udp:sendto");
            exit(1);
        }
        msg->def_count--;
    }
    printf("clock : %d\n", msg->clock);
}
