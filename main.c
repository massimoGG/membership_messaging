#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h> // for close

#include <locale.h>
#include <errno.h>

#include <stdarg.h> // for fatalError()

_Noreturn void fatalError(const char *format, ...)
{
    va_list va;
    va_start(va, format);

    // Prints this in front
    fprintf(stderr, "\u2757 ");
    vfprintf(stderr, format, va);
    va_end(va);

    // Exit
    exit(EXIT_FAILURE);
}

struct member
{
    struct sockaddr_storage node;
    struct member *next;
};

const char *decode_addr(struct sockaddr_storage *packet_socketaddr, char *packet_paddress, unsigned short *packet_port)
{
    void *packet_addr;
    int sa_family = packet_socketaddr->ss_family;

    if (sa_family == AF_INET)
    {
        packet_addr = &(((struct sockaddr_in *)packet_socketaddr)->sin_addr);
        *packet_port = ntohs(((struct sockaddr_in *)packet_socketaddr)->sin_port);
    }
    if (sa_family == AF_INET6)
    {
        packet_addr = &(((struct sockaddr_in6 *)packet_socketaddr)->sin6_addr);
        *packet_port = ntohs(((struct sockaddr_in6 *)packet_socketaddr)->sin6_port);
    }

    return inet_ntop(sa_family, packet_addr, packet_paddress, sa_family == AF_INET ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
}

/**
 * @param head The first member of the linked list
 * @param node The node to be checked
 * @return 0 if not found, 1 if found
 */
int ll_check(struct member *head, struct sockaddr_storage *node)
{
#ifdef DEBUG
    // DEBUG
    char packet_ip[INET6_ADDRSTRLEN];
    unsigned short packet_port;
    decode_addr(node, packet_ip, &packet_port);
    printf("\U0001FAE8 Checking given node: %s:%d\n", packet_ip, packet_port);

    char p_ip[INET6_ADDRSTRLEN];
    unsigned short p_port;
#endif
    // Get address of given node
    for (struct member *p = head; p != NULL; p = p->next)
    {
        // Check based on family
        if (node->ss_family != p->node.ss_family)
            continue;
#ifdef DEBUG
        decode_addr(&(p->node), p_ip, &p_port);
        printf("\t\U0001F62E %s:%d\n", p_ip, p_port);
#endif

        if (memcmp(&(p->node), node, sizeof(node)) == 0)
        {
            return 1;
        }
    }

    return 0;
}

struct member *ll_new(struct sockaddr_storage *node)
{
    struct member *new = (struct member *)calloc(1, sizeof(struct member));
    memcpy((void *)&(new->node), node, sizeof(struct member));
    new->next = NULL;
    return new;
}

struct member *ll_add(struct member *head, struct sockaddr_storage *node)
{
    struct member *p, *prev;

    for (p = head; p != NULL; p = p->next)
    {
        prev = p;
    }
    // At this point, we're at the end, p->next = NULL
    // allocate
    struct member *new = (struct member *)calloc(1, sizeof(struct member));

    memcpy(&(new->node), node, sizeof(struct member));
    new->next = NULL;

    // Append to LL
    prev->next = new;

    return new;
}

/**
 * Create UDP server and keeps listening
 */
int main(const int argc, char *argv[])
{
    // First bind socket
    setlocale(LC_ALL, "");
    const char *port = "3490";

    struct addrinfo hints = {
                        .ai_family = AF_INET6,
                        .ai_socktype = SOCK_DGRAM,
                        .ai_flags = AI_PASSIVE, // Use my IP Address
                    },
                    *servinfo, *p;

    int ret;
    if ((ret = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fatalError("Could not fetch any available address for binding!\n");
    }

    // Loop through all the results
    int sockfd = -1;

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        printf("Finding an available socket\n");
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            // Can't use this one
            continue;
        }

        // Can use this one, try to bind
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) != 0)
        {
            close(sockfd);
            int errsv = errno;
            fatalError("Could not bind socket! Errno value: %d\n", errsv);
        }

        // We reached this point, break out of loop
        break;
    }

    // If it's still -1, fatalerror out
    if (sockfd == -1)
    {
        int errsv = errno;
        fatalError("Could not create socket! Errno value: %d\n", errsv);
    }

    freeaddrinfo(servinfo); // Free linked-list

    // Array of clients

    struct member *head = NULL, *np = NULL;
#define BUFFERSIZE 1024
#define TMPBUFFERSIZE (BUFFERSIZE - INET6_ADDRSTRLEN - 32)
    char *buffer = calloc(BUFFERSIZE, sizeof(char));
    char *tmpbuffer = calloc(TMPBUFFERSIZE, sizeof(char));

    for (;;)
    {
        memset(buffer, 0, BUFFERSIZE);
        memset(tmpbuffer, 0, TMPBUFFERSIZE);

        struct sockaddr_storage packet_socketaddr;
        socklen_t paddr_len = sizeof(packet_socketaddr);

        // Blocks
        int recv_bytes = recvfrom(sockfd, tmpbuffer, TMPBUFFERSIZE, 0, (struct sockaddr *)&packet_socketaddr, &paddr_len);
        if (recv_bytes == -1)
        {
            int errsv = errno;
            fatalError("An error occured receiving packets! Errno: %d\n", errsv);
        }
        /**
         * Parse Packet source address and port
         */
        char packet_ip[INET6_ADDRSTRLEN];
        unsigned short packet_port;
        decode_addr(&packet_socketaddr, packet_ip, &packet_port);

        // Removed newline
        tmpbuffer[recv_bytes - 1] = 0; // Remove the \n from the end and replace by end-of-Line
        // Skip 1 byte messages
        if (recv_bytes == 1)
            continue;

        printf("\U0001F52C <--%db-- ([%s]:%d): '%s'\n", recv_bytes, packet_ip, packet_port, tmpbuffer);

        /**
         * Membership handling
         */
        // Check for first time
        if (head == NULL)
        {
            head = ll_new(&packet_socketaddr);
        }
        // Loop through memberships
        if (!ll_check(head, &packet_socketaddr))
        {
            ll_add(head, &packet_socketaddr);
        }

        sprintf(buffer, "\U0001F4E1 ([%s]:%d) --%db--> %s\n", packet_ip, packet_port, recv_bytes, tmpbuffer);
        size_t buflen = strlen(buffer);

        // Write to stdout
        fwrite(buffer, 1, buflen, stdout);

        // Send to all membership nodes
        for (np = head; np != NULL; np = np->next)
        {
#ifdef DEBUG
            char node_ip[INET6_ADDRSTRLEN];
            unsigned short node_port;
            decode_addr(&(np->node), node_ip, &node_port);
            printf("Broadcasting to node %s:%d\n", node_ip, node_port);
#endif
            sendto(sockfd, buffer, buflen, 0, (struct sockaddr *)&(np->node), sizeof(np->node));
        }
    }

    close(sockfd);

    return 0;
}
