#pragma once
#include <raw_message.hpp>

constexpr signed short kInvalidRange16Bit = 0x8000;

void convert(const char* file_name);
inline bool compare_msg(const gocator::Message& a, const gocator::Message& b) { return (a.timestamp < b.timestamp); }
