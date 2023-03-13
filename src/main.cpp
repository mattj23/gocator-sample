#include <algorithm>
#include <filesystem>
#include <gocator.hpp>
#include <iostream>
#include <tuple>

#define RECEIVE_TIMEOUT (20000000)
#define INVALID_RANGE_16BIT ((signed short)0x8000)
#define SENSOR_IP "192.168.1.10"

constexpr double kPointsPerMm = 1940.0 / 27.0;
constexpr double kMmPerPoint = 1.0 / kPointsPerMm;
constexpr size_t kTotalCaptures = static_cast<size_t>(kPointsPerMm * 100);

/// Gets an incrementing file name to save the data to
std::string get_file_name(size_t i, const std::string& extension) {
    std::stringstream s;
    s << "sample-";
    s << std::setw(2) << std::setfill('0') << i << extension;
    return s.str();
}

int main(int argc, char** argv) {
    gocator::Gocator sensor(SENSOR_IP);

    std::cout << "Receiving... " << std::endl;
    sensor.start_sensor();
    size_t count = 0;
    while (count++ < kTotalCaptures) {
        sensor.receive_one(RECEIVE_TIMEOUT);
    }
    sensor.stop_sensor();
    std::cout << "Done" << std::endl;

    size_t record_number = 0;
    while (std::filesystem::exists(get_file_name(record_number, ".data"))) {
        record_number++;
    }
    const auto file_name = get_file_name(record_number, ".data");

    sensor.sort_messages();

    std::cout << "Writing to " << file_name << std::endl;
    std::ofstream out_file(file_name, std::ios::out | std::ios::binary);

    for (size_t i = 0; i < sensor.messages().size(); ++i) {
        const auto& m = sensor.messages()[i];
        m.to_stream(out_file);

        for (size_t j = 0; j < m.points.size(); ++j) {
            const auto& p = m.points[j];

            if (p.x == INVALID_RANGE_16BIT) continue;
            double color = p.i / 256.0;

            // For reference, this is how you would fill the points in an Open3D point cloud from a vector of Message
            // objects.
            // point_cloud.points_.emplace_back(p.x * m.x_res + m.x_offset, i * kMmPerPoint, p.z * m.z_res + m.z_offset);
            // point_cloud.colors_.emplace_back(color, color, color);
        }
    }
    out_file.close();

    return 0;
}
