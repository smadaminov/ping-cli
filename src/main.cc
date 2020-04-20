#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "ping_cli.hpp"
#include "ping_handler.hpp"
#include "signal_handler.hpp"

// TODO:
// - add other flags
//  - set TTL
//  - interactive shell
//  - help message
//  - set up delay between icmp messages
// - use tabulate for a pretty output

PingCliProperties *PingCliProperties::properties = nullptr;

int main(int argc, char** argv) {
    // First we need to check that there is at least one argument provided.
    if (argc < 2) {
        std::cerr << "Too few arguments. For the list of available options please use --help flag." << std::endl;
        return EXIT_FAILURE;
    }

    std::string ip_address;
    int ping_sock_fd = -1;
    int ttl = 64; // According to RFC 1700 the recommended value for TTL for the IP is 64
    PingCliProperties *ping_cli = PingCliProperties::get_properties();
    ping_cli->set_sending(true);
    int icmp_next_delay = 1000; // Default delay between our pings in ms (1000ms = 1s)
    struct hostent *hostname = nullptr;
    struct timeval response_timeout;
    struct sockaddr_in target_address;

    // We need to convert the hostname into IP address and check that conversion was successfull.
    // If IP address was provided instead of hostaname, the program is still going to work fine
    // and will check that IP address.
    hostname = gethostbyname(argv[1]);
    if (hostname == nullptr) {
        std::cerr << "Cannot resolve hostname: " << argv[1] << std::endl;
        delete ping_cli;
        return EINVAL;
    }

    // Setting up `sockaddr_in` struct for the target system
    target_address.sin_family = hostname->h_addrtype;
    target_address.sin_port = 0;
    target_address.sin_addr.s_addr = *(long*)hostname->h_addr;

    ip_address = inet_ntoa(*(in_addr*)hostname->h_addr);
    // Now we are going to open the socket to send our pings. Ping (ECHO REQ-REP) is implemented
    // using ICMP protocol instead of TCP/UDP so we have to use raw sockets an ICMP protocol.
    ping_sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    // Check that we were able to open obtain a file descriptor for socket.
    if (ping_sock_fd < 0) {
        std::cerr << "Cannot open socket." << std::endl;
        delete ping_cli;
        return EXIT_FAILURE;
    }
 
    // We want to be able to stop pinging. To do that we add an interrupt handler.
    // When program will receive SIGINT ([CTRL]+C) from user, it will stop.
    signal(SIGINT, signal_handler);

    // Let's set up TTL for our ping socket
    if (setsockopt(ping_sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        std::cerr << "Cannot set up TTL." << std::endl;
        delete ping_cli;
        close(ping_sock_fd);
        return EXIT_FAILURE;
    }

    // Let's set up timeout for recv() to avoid getting stuck waiting for ICMP reply.
    response_timeout.tv_sec = 1; // Wait one second for recv timeout
    response_timeout.tv_usec = 0;
    if (setsockopt(ping_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &response_timeout, sizeof(response_timeout)) < 0) {
        std::cerr << "Cannot set up response timeout." << std::endl;
        delete ping_cli;
        close(ping_sock_fd);
        return EXIT_FAILURE;
    }

    // This is out main routine -- pinging the given hostname/IP until user will
    // decide it's enough by sending SIGINT signal to the program.
    while(ping_cli->is_sending()) {
        if (send_another_ping(ping_sock_fd, target_address) < 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(icmp_next_delay));
    }

    // Let's develop a good habbit of cleaning up after ourselves.
    // In our case, we need to delete singleton instance and close socket.
    delete ping_cli;
    close(ping_sock_fd);

    return EXIT_SUCCESS;
}
