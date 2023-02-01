#pragma once

#include <GoSdk/GoSdk.h>

#include <string>
#include <vector>

#include "raw_message.hpp"

namespace gocator {
class GocatorError : public std::exception {
public:
    GocatorError(const char* message, int32_t status) : message_(message), status_(status) {}
    const char* what() { return message_; }

private:
    const char* message_;
    int32_t status_;
};

class Gocator {
public:
    explicit Gocator(const std::string& ip_addr);
    ~Gocator();

    Gocator(const Gocator&) = delete;
    Gocator(Gocator&&) = delete;
    Gocator& operator=(const Gocator&) = delete;
    Gocator& operator=(Gocator&&) = delete;

    void stop_sensor();
    void start_sensor();

    bool receive(GoDataSet* data_set, uint64_t timeout);

private:
    kAssembly api_{};
    GoSystem system_{};
    GoSensor sensor_{};
    GoSetup setup_{};
    size_t point_width_{};
};

}  // namespace gocator