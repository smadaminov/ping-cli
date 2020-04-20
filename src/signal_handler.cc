#include <iostream>
#include <csignal>
#include "signal_handler.hpp"
#include "ping_cli.hpp"

void signal_handler(int signal_number) {
    PingCliProperties *ping_cli = PingCliProperties::get_properties();
    if (signal_number == SIGINT) {
        std::cout << "Received interrupt. Stopping..." << std::endl;
        ping_cli->set_sending(false);
    }
}
