#pragma once
#include <open3d/Open3D.h>

#include <optional>
#include <raw_message.hpp>
#include <unordered_map>

constexpr signed short kInvalidRange16Bit = 0x8000;
constexpr size_t kMax16Bit = 0xFFFF;

void convert(const char* file_name);
inline bool compare_msg(const gocator::Message& a, const gocator::Message& b) { return (a.timestamp < b.timestamp); }
