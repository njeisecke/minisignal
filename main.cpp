#include "signal.h"

#include <iostream>

struct Receiver
{
    int id;
    std::unique_ptr<Connection> connection;

    void onMessage(const std::string &msg)
    {
        std::cout << "Receiver " << id << " got: " << msg << std::endl;
    }
};

int main()
{
    Signal<const std::string&> transmitter;

    {
        Receiver rx1 { .id = 1 };
        rx1.connection = transmitter.connect([&](const std::string &m) { rx1.onMessage(m); });
        // these all replace the previous connection...
        rx1.connection = transmitter.connect([&](const std::string &m) { rx1.onMessage(m); });
        rx1.connection = transmitter.connect([&](const std::string &m) { rx1.onMessage(m); });
        // ... last one winning
        rx1.connection = transmitter.connect([&](const std::string &m) { rx1.onMessage(m + " foo"); });

        // example for using a ConnectionGroup
        ConnectionGroup group;
        transmitter.connect(group, [&](const std::string &m) { std::cout << "context receiver 1: " << m << std::endl; });
        transmitter.connect(group, [&](const std::string &m) { std::cout << "context receiver 2: " << m << std::endl; });
        transmitter.connect(group, [&](const std::string &m) { std::cout << "context receiver 3: " << m << std::endl; });

        // oops, this will give a warning for not storing the result
        transmitter.connect([&](const std::string &m) { rx1.onMessage(m); });

        Receiver rx2 { .id = 2 };
        rx2.connection = transmitter.connect([&](const std::string &m) { rx2.onMessage(m); });

        Receiver rx3 { .id = 3 };
        rx3.connection = transmitter.connect([&](const std::string &m) { rx3.onMessage(m); });

        // rx1, rx2, rx3 alive
        transmitter.emit("First");

        // explicit disconnect
        std::cout << "rx2 connected " << rx2.connection->isConnected() << std::endl;
        rx2.connection->disconnect();
        std::cout << "rx2 connected " << rx2.connection->isConnected() << std::endl;

        transmitter.emit("Second"); // rx2 disconnect

    } // rx1 + rx2 + rx3 + group go out of scope here, all connections destroyed

    transmitter.emit("Third (no receiver)"); // No one receives this

    return 0;
}
