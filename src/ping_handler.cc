#include <iostream>
#include <array>
#include <numeric>
#include <algorithm>
#include <chrono>

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

    int bytes = -1;
    int addr_len;
    unsigned char buf[255];
    struct sockaddr_in addr;
    struct iphdr *ip = nullptr;
    icmphdr *header;

    bzero(&addr, sizeof(addr));
    addr_len = sizeof(addr);

    bzero(&pkt, sizeof(pkt));
    pkt.header.type = ICMP_ECHO;
    pkt.header.un.echo.id = getpid();

    // Let's do C++ Lambda expression because `why not?`
    std::for_each(pkt_size.begin(), pkt_size.end(), [&pkt](int i){ pkt.msg[i - 1] = (i - 1) + '0'; });
    pkt.msg[pkt_size.size()] = 0;
    pkt.header.un.echo.sequence = ping_cli->get_tx_message_count();
    pkt.header.checksum = checksum(&pkt, sizeof(pkt));

    ping_cli->inc_tx_message_count();

    auto start = std::chrono::steady_clock::now();

    if (sendto(ping_sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr*)&target_address, sizeof(target_address)) <= 0) {
        std::cerr << "Cannot send ICMP message." << std::endl;
        return -1;
    }

    if ((bytes = recvfrom(ping_sock_fd, &buf, sizeof(buf), 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len)) <= 0) {
        if (ping_cli->is_sending()) {
            std::cerr << "Failed during receiving ICMP message." << std::endl;
        }
        ip = (struct iphdr*)buf;
        header = (struct icmphdr*)(buf + ip->ihl*4);

        // Type 0 is ICMP_ECHOREPLY.
        // Code 0 is code for unreachable.
        if (header->type == 0 && header->code == 0) {
            std::cerr << "Unreachable." << std::endl;
        }

        return -1;
    }

    auto end = std::chrono::steady_clock::now();

    ping_cli->inc_rx_message_count();

    ip = (struct iphdr*)buf;
    header = (struct icmphdr*)(buf + ip->ihl*4);

    auto id_check = pkt.header.un.echo.id == header->un.echo.id;
    auto sequence_check = pkt.header.un.echo.sequence == header->un.echo.sequence;

    // TODO: There are cases when this check (RFC 792) fails:
    // - e.g., previous messages timed out. Need to be fixed.
    if (!(id_check && sequence_check)) {
        std::cerr << "RFC 792 violation." << std::endl;
    }

    std::cout << "===== SENT: " << ping_cli->get_tx_message_count() << " ===== RECEIVED: " << ping_cli->get_rx_message_count() << " =====" << std::endl;
    std::cout << "RTT  : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "us" << std::endl;
    std::cout << "Loss : " << 100.0 * (ping_cli->get_tx_message_count() - ping_cli->get_rx_message_count()) / ping_cli->get_tx_message_count() << "%" << std::endl;
    std::cout << "=====================================" << std::endl;

    return 0;
}
