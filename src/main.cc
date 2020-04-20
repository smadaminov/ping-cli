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
#include <strings.h>
#include <string.h>

#include "ping_cli.hpp"
#include "ping_handler.hpp"
#include "signal_handler.hpp"
#include "icmp_recv.hpp"

PingCliProperties *PingCliProperties::properties = nullptr;

int main(int argc, char** argv) {
    std::string ip_address;
    int ping_sock_fd = -1;
    int ttl = 64; // According to RFC 1700 the recommended value for TTL for the IP is 64
    PingCliProperties *ping_cli = PingCliProperties::get_properties();
    ping_cli->set_sending(true);
    int icmp_next_delay = 4000; // Default delay between our pings in ms (1000ms = 1s)
    struct hostent *hostname = nullptr;
    struct sockaddr_in target_address;
    struct timeval response_timeout;

    // First we need to check that there is at least one argument provided.
    if (argc < 2) {
        std::cerr << "Too few arguments. For the list of available options please use -h flag." << std::endl;
        delete ping_cli;
        return EXIT_FAILURE;
    }

    char hostname_arg[255];
    strcpy(hostname_arg, argv[1]);

    int c, ttl_new, delay;
    while((c = getopt(argc, argv, "t:d:h")) != -1) {
        switch(c) {
            case 't':
                ttl_new = atoi(optarg);
                if (ttl_new < 1 && ttl_new > 255) {
                    std::cerr << "Wrong TTL value. It should be between 1 and 255." << std::endl;
                    return EINVAL;
                }
                ttl = ttl_new;
                break;
            case 'd':
                delay = atoi(optarg);
                if (delay < 2) {
                    std::cerr << "Wrong delay value. It should be greater than 2s." << std::endl;
                }
                icmp_next_delay = delay * 1000;
                break;
            case 'h':
                std::cout << "You can run this program as follows: ";
                std::cout << "sudo ./bin/ping-cli hostname|IP [-t TTL] [-d DELAY]" << std::endl;
                return EXIT_SUCCESS;
                break;
            default:
                std::cerr << "Unsupported argument found." << std::endl;
                return EINVAL;
        }
    }

    // We need to convert the hostname into IP address and check that conversion was successfull.
    // If IP address was provided instead of hostaname, the program is still going to work fine
    // and will check that IP address.
    hostname = gethostbyname(hostname_arg);
    if (hostname == nullptr) {
        std::cerr << "Cannot resolve hostname: " << hostname_arg << std::endl;
        delete ping_cli;
        return EINVAL;
    }

    // Setting up `sockaddr_in` struct for the target system
    bzero(&target_address, sizeof(target_address));
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
    if (setsockopt(ping_sock_fd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        std::cerr << "Cannot set up TTL." << std::endl;
        delete ping_cli;
        close(ping_sock_fd);
        return EXIT_FAILURE;
    }

    // Setting up timeout to avoid getting stuck in the recv().
    response_timeout.tv_sec = 1; // Wait one second for recv timeout
    response_timeout.tv_usec = 0;
    if (setsockopt(ping_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &response_timeout, sizeof(response_timeout)) < 0) {
        std::cerr << "Cannot set up response timeout." << std::endl;
        return -1;
    }

    // TODO: Implement receiving part as a separate thread
    // Spawning a dedicated thread to listen to incoming ICMP messages.
    // It is partially needed to avoid getting stuck in recv() and still keep
    // sending new ICMP requests with approximately same delay.
    // std::thread icmp_recv_th(icmp_recv);

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
    // TODO: Once we move to separate thread for incoming ICMP messages
    // TODO: we will need to join this thread `icmp_recv_th.join();`

    delete ping_cli;
    close(ping_sock_fd);

    return EXIT_SUCCESS;
}
