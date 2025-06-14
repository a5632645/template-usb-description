#pragma once
#include "constexpr_usb.hpp"
#include <cstddef>
#include <cstdint>

namespace usbpp::uac {

struct BaseClock {
    CharArray<8> char_array;

    constexpr BaseClock() {
        
    }
};

template<size_t N>
struct BaseAudioFunction {
    CharArray<N> char_array;

    constexpr BaseAudioFunction<N>& SetBCDADC(uint16_t bcd) {
        char_array[3] = bcd & 0xff;
        char_array[4] = bcd >> 8;
    }
    constexpr uint8_t& Catalog() { return char_array[5]; }
    constexpr BaseAudioFunction<N>& SetTotalLength(uint16_t len) {
        char_array[6] = len & 0xff;
        char_array[7] = len >> 8;
    }
    constexpr uint8_t& Controls() { return char_array[8]; }

    constexpr BaseAudioFunction() {
        char_array[0] = 9;
        char_array[1] = 0x24;
        char_array[2] = 1;
    }


    template<size_t N2>
    constexpr BaseAudioFunction<N + N2> AddClock() {

    }
};
using AudioFunction = BaseAudioFunction<9>;

template<size_t N>
struct AudioControlInterface : public BaseInterface<N> {
    constexpr AudioControlInterface() {
        this->ClassNO() = 1;
        this->Protocol() = 0x20;
    }

    template<size_t N2>
    constexpr AudioControlInterface<N + N2> AddAudioFunction(BaseAudioFunction<N2> function) {
        AudioControlInterface<N + N2> ret;
        ret.char_array.MergeArray(this->char_array, function.char_array);
        return ret;
    }
};

}