#include <iostream>
#include <vector>
#include <algorithm>

#include <open3d/Open3D.h>
#include <main.hpp>

constexpr double kSpacing = (27.0 / 1940.0);


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Provide path of file to convert" << std::endl;
        return 1;
    }
    for (size_t i = 1; i < argc; ++i) {
        std::cout << "Converting " << argv[i] << std::endl;
        convert(argv[i]);
    }
    return 0;
}

void convert(const char* file_name) {
    using namespace open3d;

    std::string base_path{file_name};

    std::ifstream in_file(file_name, std::ios::binary | std::ios::in);
    std::vector<gocator::Message> recorded;

    size_t rows = 0;
    while (!in_file.eof()) {
        auto msg = gocator::Message::from_stream(in_file);
        if (msg.timestamp > 0) {
            recorded.push_back(msg);
            rows = std::max(rows, msg.points.size());
        }
    }
    std::sort(recorded.begin(), recorded.end(), compare_msg);

    geometry::PointCloud cloud;

    // Mapping between indices and intensities


    for (size_t i = 0; i < recorded.size(); ++i) {
        const auto& m = recorded[i];
        double y_pos = i * kSpacing;

        for (size_t k = 0; k < m.points.size(); ++k) {
            const auto& p = m.points[k];
            if (p.x == kInvalidRange16Bit) continue;

            cloud.points_.emplace_back(p.x * m.x_res + m.x_offset, y_pos, p.z * m.z_res + m.z_offset);
            double c = p.i / 256.0;
            cloud.colors_.emplace_back(c, c, c);
        }
    }

    // Full point cloud
    open3d::io::WritePointCloudOption option(false, true);
    open3d::io::WritePointCloudToPCD(base_path + ".full.pcd", cloud, option);
}
