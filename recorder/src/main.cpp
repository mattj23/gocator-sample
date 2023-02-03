#include <open3d/geometry/PointCloud.h>
#include <open3d/io/PointCloudIO.h>

#include <algorithm>
#include <filesystem>
#include <gocator.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>

#define RECEIVE_TIMEOUT (20000000)
#define INVALID_RANGE_16BIT ((signed short)0x8000)
#define SENSOR_IP "192.168.1.10"

constexpr double kPointsPerMm = 1940.0 / 27.0;
constexpr double kMmPerPoint = 1.0 / kPointsPerMm;
constexpr size_t kTotalCaptures = static_cast<size_t>(kPointsPerMm * 100);

std::string get_file_name(size_t i, const std::string& extension) {
    std::stringstream s;
    s << "sample-";
    s << std::setw(2) << std::setfill('0') << i << extension;
    return s.str();
}

int main(int argc, char** argv) {
    using namespace open3d;

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

//    auto point_cloud = std::make_shared<open3d::geometry::PointCloud>();
    geometry::PointCloud point_cloud{};

    std::cout << "Writing to " << file_name << std::endl;
    std::ofstream out_file(file_name, std::ios::out | std::ios::binary);

    std::vector<std::tuple<uint32_t, uint32_t>> point_indices;
    for (size_t i = 0; i < sensor.messages().size(); ++i) {
        const auto& m = sensor.messages()[i];
        m.to_stream(out_file);

        for (size_t j = 0; j < m.points.size(); ++j) {
            const auto& p = m.points[j];

            if (p.x == INVALID_RANGE_16BIT) continue;
//            point_cloud.points_.emplace_back(p.x * m.x_res + m.x_offset, i * kMmPerPoint, p.z * m.z_res + m.z_offset);
            double color = p.i / 256.0;
//            point_cloud.colors_.emplace_back(color, color, color);
            point_indices.emplace_back(i, j);
        }
    }
    out_file.close();

//    io::WritePointCloudOption options(true);
//    io::WritePointCloudToPLY(get_file_name(record_number, ".ply"), point_cloud, options);

    return 0;
}
