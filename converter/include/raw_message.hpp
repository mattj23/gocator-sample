#pragma once

#include <cinttypes>
#include <fstream>
#include <vector>

namespace gocator {
constexpr signed short kInvalidRange = ((signed short)0x8000);

struct Point {
    int16_t x{0};
    int16_t z{0};
    uint8_t i{0};
};

struct Message {
    uint64_t timestamp{0};
    uint64_t frame_index{0};
    uint64_t encoder{0};

    double x_res{0};
    double z_res{0};
    double x_offset{0};
    double z_offset{0};

    std::vector<Point> points;

    void to_stream(std::ofstream& file) const;
    static Message from_stream(std::ifstream& file);
};

struct Correspondence {
    uint16_t row;
    uint16_t col;
    uint8_t i;

    void to_stream(std::ofstream& file) const;
};
}  // namespace gocator