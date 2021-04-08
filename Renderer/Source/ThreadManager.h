#pragma once

#include <thread>
#include <mutex>

class ThreadGuard {
public:
    enum class DesAction { JOIN, DETACH };

    ThreadGuard(std::thread&& t, DesAction a) : t(std::move(t)), action(a) {};

    ~ThreadGuard() {
        if (t.joinable()) {
            if (action == DesAction::JOIN) {
                t.join();
            }
            else {
                t.detach();
            }
        }
    }

    ThreadGuard(ThreadGuard&&) = default;
    ThreadGuard& operator=(ThreadGuard&&) = default;

    std::thread& get() { return t; }

private:
    std::thread t;
    DesAction action;
};