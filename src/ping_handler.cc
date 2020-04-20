#include <iostream>
#include <array>
#include <numeric>
#include <algorithm>

#include <sys/types.h>
#include <unistd.h>
#include <strings.h>

#include "ping_handler.hpp"
#include "ping_cli.hpp"
#include "checksum.hpp"

// Let's not create a copy of a descriptor but rather pass it by reference
int send_another_ping(const int& ping_sock_fd, const struct sockaddr_in& target_address) {
    PingCliProperties *ping_cli = PingCliProperties::get_properties();

    icmp_pkt pkt;
    std::array<int, sizeof(pkt.msg) - 1> pkt_size;
    std::iota(pkt_size.begin(), pkt_size.end(), 1);

    bzero(&pkt, sizeof(pkt));
    pkt.header.type = ICMP_ECHO;
    pkt.header.un.echo.id = getpid();

    // Let's do C++ Lambda expression becease `why not?`
    std::for_each(pkt_size.begin(), pkt_size.end(), [&pkt](int i){ pkt.msg[i - 1] = (i - 1) + '0'; });
    pkt.msg[pkt_size.size()] = 0;
    pkt.header.un.echo.sequence = ping_cli->get_message_count();
    pkt.header.checksum = checksum(&pkt, sizeof(pkt));

    ping_cli->inc_message_count();

    if (sendto(ping_sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&target_address, sizeof(target_address)) <= 0) {
        std::cerr << "Cannot send ICMP message." << std::endl;
        return -1;
    }
}
