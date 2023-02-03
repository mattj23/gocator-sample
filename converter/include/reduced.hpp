#pragma once
#include <Eigen/Dense>
#include <optional>

class Reduced {
public:
    void add(size_t row, size_t col, Eigen::Vector3d vec);
    std::optional<Eigen::Vector3d> at(size_t row, size_t col) const;

    inline size_t max_row() const { return max_row_; }
    inline size_t max_col() const { return max_col_; }

private:
    std::unordered_map<size_t, std::unordered_map<size_t, Eigen::Vector3d>> internal_;
    size_t max_row_{};
    size_t max_col_{};
};
