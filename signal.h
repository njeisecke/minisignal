#pragma once

#include <memory>
#include <functional>

#include "connection.h"

/// Any type that satisfies this concept can be used to simplify storing of connections,
/// avoiding the need to manage a unique_ptr for each established connection.
template <typename T>
concept IsConnectionContext = requires(T t, std::unique_ptr<Connection> &&c) {
    { t.storeConnection(std::move(c)) } -> std::same_as<void>;
};

/// Manages connections in a group context, preventing individual disconnects.
/// @implements IsConnectionContext
class ConnectionGroup {
    std::vector<std::unique_ptr<Connection>> connections;

public:
    void storeConnection(std::unique_ptr<Connection> &&connection) {
        this->connections.push_back(std::move(connection));
    }

    void disconnectAll() {
        connections.clear();
    }
};

template <typename... Args>
class Signal
{
    // Callback + Connection Tracking
    struct Bond
    {
        std::function<void(Args...)> callback;
        std::weak_ptr<void> weakTracker; // Tracks if the Connection still exists
    };

    // mutable because we must remove dead receivers in the const emit method
    mutable std::vector<Bond> bonds;

public:

    // Connects a function and returns a Connection object to the receiver
    [[nodiscard]] std::unique_ptr<Connection> connect(std::function<void(Args...)> &&callback)
    {
        auto connection = std::make_unique<Connection>();
        // Store the callback and a weak reference to the Connection's Tracker
        bonds.push_back({ std::move(callback), connection->tracker });

        // Something else must store the Connection, we only watch its Tracker
        return connection;
    }

    // Connects a function and stores the connection in some context
    void connect(IsConnectionContext auto &context, std::function<void(Args...)> &&callback)
    {
        // transfer owenership to the context
        context.storeConnection(connect(std::move(callback)));
    }

    // Emit the signal.
    void emit(Args... args) const
    {
        // this will clean up alls dead receivers on the go.
        for (auto it = bonds.begin(); it != bonds.end(); ) {
            // Check if the receiver's connection handle is still alive/active
            if (const auto s = it->weakTracker.lock()) {
                it->callback(args...);
                ++it;
            } else {
                // Sender-side cleanup: Receiver is dead, remove the bond
                it = bonds.erase(it);
            }
        }
    }

    // sugar - but I prefer the explicit emit to make clear a signal is invoked
    // void operator()(Args... args) const { emit(args...); }
};
