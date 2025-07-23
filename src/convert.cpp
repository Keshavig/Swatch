#include "convert.h"
#include <cstdint>

size_t simple_power(const int base, const int power) {
    size_t result = 1;

    if (power < 0) return 0;
    else if (power == 0 && base != 0) return 1;

    for (size_t i = 1; i <= power; ++i) {
        result *= base;
    }

    return result;
}

/* This does not check for any mistakes!!! */
size_t hex2dec(const std::string_view sv) {
    const size_t len = sv.length();
    size_t number = 0;

    for (int i = (len-1); i >= 0; --i) {
        int current_digit = 0;
        switch (sv[i]) {
            case 'A': case 'a': current_digit = 10; break;
            case 'B': case 'b': current_digit = 11; break;
            case 'C': case 'c': current_digit = 12; break;
            case 'D': case 'd': current_digit = 13; break;
            case 'E': case 'e': current_digit = 14; break;
            case 'F': case 'f': current_digit = 15; break;
            default: current_digit = sv[i] - '0';
        }

        number += current_digit * simple_power(16, (len-1) - i);
    }

    return number;
}


ImVec4 colorV4(const std::string_view hex_code, float opacity) {
    assert(hex_code.length() == 7 && hex_code[0] == '#');

    const char red  [3]   = { hex_code[1], hex_code[2] };
    const char green[3]   = { hex_code[3], hex_code[4] };
    const char blue [3]   = { hex_code[5], hex_code[6] };

    float red_   = static_cast<float>(hex2dec(red)) / 255;
    float green_ = static_cast<float>(hex2dec(green)) / 255;
    float blue_  = static_cast<float>(hex2dec(blue)) / 255;

    return ImVec4(red_, green_, blue_, opacity);
}


ImU32 colorU32(const std::string_view hex_code, ImU8 opacity) {
    assert(hex_code.length() == 7 && hex_code[0] == '#');

    const char red  [3]   = { hex_code[1], hex_code[2] };
    const char green[3]   = { hex_code[3], hex_code[4] };
    const char blue [3]   = { hex_code[5], hex_code[6] };

    ImU8 red_   = static_cast<ImU8>(hex2dec(red));
    ImU8 green_ = static_cast<ImU8>(hex2dec(green));
    ImU8 blue_  = static_cast<ImU8>(hex2dec(blue));

    return IM_COL32(red_, green_, blue_, opacity);
}
