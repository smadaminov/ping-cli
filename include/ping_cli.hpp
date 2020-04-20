#ifndef __PING_CLI_HPP
#define __PING_CLI_HPP

#include <mutex>

// We create a singleton class to store some of the shared state and properties
// of our ping-cli application.
class PingCliProperties {
    public:
        static PingCliProperties *get_properties();
        bool is_sending();
        void set_sending(bool keep_sending);
        int get_message_count();
        void inc_message_count();

    private:
        static PingCliProperties *properties;
        bool keep_sending;
        int message_count;
        std::mutex keep_sending_lock; // If I ever go multithreaded may become handy
        PingCliProperties();
};

#endif /* __PING_CLI_HPP */
