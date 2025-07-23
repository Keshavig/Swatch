#pragma once
#include <string_view>
#include "imgui.h"

size_t simple_power(const int base, const int power);
size_t hex2dec(const std::string_view sv);

ImVec4 colorV4(const std::string_view hex_code, float opacity = 1.0f);
ImU32 colorU32(const std::string_view hex_code, ImU8 opacity = 255);
