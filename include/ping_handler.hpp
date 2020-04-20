#ifndef __PING_HANDLER_HPP
#define __PING_HANDLER_HPP

#include <netinet/ip_icmp.h>

#define ICMP_PKT_SIZE 64

typedef struct icmp_pkt {
    struct icmphdr header;
    char msg[ICMP_PKT_SIZE - sizeof(struct icmphdr)];
} icmp_pkt;

int send_another_ping(const int& ping_sock_fd, const struct sockaddr_in& target_address);

#endif /* __PING_HANDLER_HPP */
