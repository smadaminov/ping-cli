#include "ping_cli.hpp"

PingCliProperties* PingCliProperties::get_properties() {
    if (properties == nullptr) {
        properties = new PingCliProperties();
    }

    return properties;
}

bool PingCliProperties::is_sending() {
    const std::lock_guard<std::mutex> lock(keep_sending_lock);
    return keep_sending;
}

void PingCliProperties::set_sending(bool keep_sending) {
    const std::lock_guard<std::mutex> lock(keep_sending_lock);
    this->keep_sending = keep_sending;
}

int PingCliProperties::get_tx_message_count() {
    return this->tx_message_count;
}

void PingCliProperties::inc_tx_message_count() {
    this->tx_message_count++;
}

int PingCliProperties::get_rx_message_count() {
    return this->rx_message_count;
}

void PingCliProperties::inc_rx_message_count() {
    this->rx_message_count++;
}

void PingCliProperties::terminate() {
    const std::lock_guard<std::mutex> lock(keep_sending_lock);
    this->keep_sending = false;
}

PingCliProperties::PingCliProperties() : tx_message_count(0), rx_message_count(0) { }
