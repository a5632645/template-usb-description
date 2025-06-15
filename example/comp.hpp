#pragma once
#include <cstdint>

struct CompareResult {
    bool diff = false;
    uint32_t index = 0;
    uint8_t a = 0;
    uint8_t b = 0;
};
template<size_t N1>
static constexpr CompareResult Compare(const uint8_t(&a)[N1], const uint8_t(&b)[N1]) {
    CompareResult res;
    for (size_t i = 0; i < N1; ++i) {
        if (a[i] != b[i]) {
            res.diff = true;
            res.index = i;
            res.a = a[i];
            res.b = b[i];
            break;
        }
    }
    return res;
}
