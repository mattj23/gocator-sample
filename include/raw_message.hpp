#pragma once

#include <cinttypes>
#include <vector>

namespace gocator {
constexpr signed short kInvalidRange = ((signed short)0x8000);

struct Point {
    int16_t x;
    int16_t y;
    uint8_t i;
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
};
}  // namespace gocator