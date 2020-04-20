#include <iostream>

#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>

#include "icmp_recv.hpp"
#include "ping_cli.hpp"
#include "ping_handler.hpp"

void icmp_recv() {
    int recv_sock_fd = -1;
    int addr_len;
    struct sockaddr_in addr;
    struct timeval response_timeout;
    icmp_pkt pkt;
   
    PingCliProperties *ping_cli = PingCliProperties::get_properties();

    // No reason to start receiving reply messages when none was sent.
    while(ping_cli->get_tx_message_count() == 0) { }

    if ((recv_sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        std::cerr << "Cannot open receiving socket." << std::endl;
        ping_cli->terminate();
        return ;
    }

    // Let's set up timeout for recv() to avoid getting stuck waiting for ICMP reply.
    response_timeout.tv_sec = 1; // Wait one second for recv timeout
    response_timeout.tv_usec = 0;
    if (setsockopt(recv_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &response_timeout, sizeof(response_timeout)) < 0) {
        std::cerr << "Cannot set up response timeout." << std::endl;
        close(recv_sock_fd);
        ping_cli->terminate();
        return ;
    }

    while(ping_cli->is_sending()) {
        bzero(&pkt, sizeof(pkt));
        bzero(&addr, sizeof(addr));
        addr_len = sizeof(addr);
        if (recvfrom(recv_sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len) <= 0) {
            if (ping_cli->is_sending()) {
                std::cerr << "Failed during receiving ICMP message." << std::endl;
                close(recv_sock_fd);
                ping_cli->terminate();
                return ;
            } else {
                close(recv_sock_fd);
                return ;
            }
        }

        std::cout << "Received reply..." << std::endl;
        std::cout << "I: " << pkt.header.un.echo.id << std::endl;
        std::cout << "S: " << pkt.header.un.echo.sequence << std::endl;
        std::cout << "type and code: " << pkt.header.type << " " << pkt.header.code << std::endl;
    }

    close(recv_sock_fd);
}
