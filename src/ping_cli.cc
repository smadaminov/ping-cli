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

int PingCliProperties::get_message_count() {
    return this->message_count;
}

void PingCliProperties::inc_message_count() {
    this->message_count++;
}

PingCliProperties::PingCliProperties() { }
