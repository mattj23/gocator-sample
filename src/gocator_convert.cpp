#include <iostream>
#include <gocator.hpp>

constexpr double kPointsPerMm = 1940.0 / 27.0;
constexpr double kMmPerPoint = 1.0 / kPointsPerMm;
#define INVALID_RANGE_16BIT ((signed short)0x8000)

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Error: must provide argument of data file to deserialize.";
        return -1;
    }

    std::ifstream in_file(argv[1], std::ios::in | std::ios::binary);
    if (!in_file.is_open()) {
        std::cout << "Error opening file: " << argv[1] << std::endl;
        return -1;
    }

    size_t row = 0;
    while (!in_file.eof()) {
        auto m = gocator::Message::from_stream(in_file);
        for (size_t col = 0; col < m.points.size(); ++col) {
            const auto& p = m.points[col];
            if (p.x == INVALID_RANGE_16BIT) continue;

            // In this case we'll use the message index to determine the y position, which assumes that the sensor was
            // moving at a constant speed and you know the spacing between the rows.  Most likely you'll want to use
            // the encoder value if you have it.
            double y = row * kMmPerPoint;

            double x = p.x * m.x_res + m.x_offset;
            double z = p.z * m.z_res + m.z_offset;
            double color = p.i / 256.0;
            std::cout << x << ", " << y << ", " << z << ", " << color << "\n";
        }

        row++;
    }

    return 0;
}