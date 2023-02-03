#include <reduced.hpp>

void Reduced::add(size_t row, size_t col, Eigen::Vector3d vec) {
    internal_[row][col] = vec;
    max_col_ = std::max(max_col_, col);
    max_row_ = std::max(max_row_, row);
}

std::optional<Eigen::Vector3d> Reduced::at(size_t row, size_t col) const {
    if (!internal_.contains(row)) return {};
    if (!internal_.at(row).contains(col)) return {};

    return internal_.at(row).at(col);
}
