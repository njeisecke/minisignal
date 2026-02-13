#pragma once

#include <memory>

// Used to manage the lifetime of a connection. A Connection is
// unique.
class Connection
{
    // A std::weak_ptr of Tracker is hold by the signal, so it can detect
    // destroyed connections. We do not want the Connection itself to be shareable,
    // it must be unique, so use this empty helper struct.
    struct Tracker {};
    std::shared_ptr<Tracker> tracker = std::make_shared<Tracker>();

    template <typename... Args>
    friend class Signal;

public:
    // automatically disconnects when the handle dies
    ~Connection() {}

    // explicit disconnect
    void disconnect() {
        tracker.reset();
    }

    bool isConnected() const {
        return tracker.get();
    }

    // Allow moving, but prevent accidental copying to keep lifetime clear
    Connection() = default;
    Connection(const Connection&) = delete;
    Connection(Connection&&) = default;
    Connection& operator=(Connection&&) = default;
};
