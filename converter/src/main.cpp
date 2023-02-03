#include <algorithm>
#include <iostream>
#include <main.hpp>
#include <reduced.hpp>
#include <vector>

constexpr double kSpacing = (27.0 / 1940.0);  // mm per point
constexpr double kReducedSpacing = 0.1;       // mm
constexpr double kReducedTolDistance = kReducedSpacing * 1.75;
constexpr size_t kSkipCount = kReducedSpacing / kSpacing;

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
    std::vector<Correspondence> correspondences;
    Reduced reduced;

    if (recorded.size() > kMax16Bit) {
        std::cout << "Too many rows to fit in the uint16_t size: " << recorded.size() << std::endl;
        throw std::exception();
    }

    for (size_t i = 0; i < recorded.size(); ++i) {
        const auto& m = recorded[i];
        double y_pos = static_cast<double>(i) * kSpacing;

        for (size_t k = 0; k < m.points.size(); ++k) {
            const auto& p = m.points[k];
            if (p.x == kInvalidRange16Bit) continue;

            Eigen::Vector3d v{p.x * m.x_res + m.x_offset, y_pos, p.z * m.z_res + m.z_offset};
            cloud.points_.push_back(v);
            double c = p.i / 256.0;
            cloud.colors_.emplace_back(c, c, c);

            correspondences.push_back({static_cast<uint16_t>(i), static_cast<uint16_t>(k), p.i});

            // Reduced points
            if (i % kSkipCount == 0 && k % kSkipCount == 0) {
                size_t i_ = i / kSkipCount;
                size_t k_ = k / kSkipCount;

                reduced.add(i_, k_, v);
            }
        }
    }

    // Full point cloud
    open3d::io::WritePointCloudOption option(false, true);
    open3d::io::WritePointCloudToPCD(base_path + ".full.pcd", cloud, option);

    // Partial point cloud
    geometry::TriangleMesh thinned;
    for (size_t i = 0; i < reduced.max_row() - 1; ++i) {
        for (size_t k = 0; k < reduced.max_col() - 1; ++k) {
            auto p0 = reduced.at(i, k);
            auto p1 = reduced.at(i + 1, k);
            auto p2 = reduced.at(i + 1, k + 1);
            auto p3 = reduced.at(i, k + 1);

            if (!(p0.has_value() && p1.has_value() && p2.has_value() && p3.has_value())) continue;

            if ((p0.value() - p1.value()).norm() > kReducedTolDistance ||
                (p0.value() - p2.value()).norm() > kReducedTolDistance ||
                (p0.value() - p3.value()).norm() > kReducedTolDistance)
                continue;

            size_t v0 = thinned.vertices_.size();
            thinned.vertices_.push_back(p0.value());
            thinned.vertices_.push_back(p1.value());
            thinned.vertices_.push_back(p2.value());
            thinned.vertices_.push_back(p3.value());

            thinned.triangles_.emplace_back(v0 + 2, v0 + 1, v0);
            thinned.triangles_.emplace_back(v0 + 3, v0 + 2, v0);
        }
    }

    thinned.ComputeVertexNormals();
    open3d::io::WriteTriangleMeshToSTL(base_path + ".thinned.stl", thinned, false, false, true, false, false, false);
}
