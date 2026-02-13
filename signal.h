#pragma once

#include <memory>
#include <functional>

#include "connection.h"

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
