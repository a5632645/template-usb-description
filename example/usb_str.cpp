#include "tpusb/usb.hpp"
#include <cstddef>

template<class CharType, size_t len, const CharType (&str)[len]>
struct CompileTimeUtf8ToUnicodeLength {
    template<size_t counter, size_t offset>
    static constexpr size_t GetUnicodeLength2() {
        if constexpr (offset == len) {
            return counter - 1;
        }
        else if constexpr (offset > len) {
            throw "not utf8";
            return counter;
        }
        else if constexpr ((str[offset] & 0x80) == 0) {
            // 1-byte UTF-8 character (ASCII)
            return GetUnicodeLength2<counter + 1, offset + 1>();
        } else if constexpr ((str[offset] & 0xE0) == 0xC0) {
            // 2-byte UTF-8 character
            return GetUnicodeLength2<counter + 1, offset + 2>();
        } else if constexpr ((str[offset] & 0xF0) == 0xE0) {
            // 3-byte UTF-8 character
            return GetUnicodeLength2<counter + 1, offset + 3>();
        } else if constexpr ((str[offset] & 0xF8) == 0xF0) {
            // 4-byte UTF-8 character
            return GetUnicodeLength2<counter + 1, offset + 4>();
        } else {
            // Invalid UTF-8 start byte
            throw "not utf8";
            return counter;
        }
    }

    static constexpr size_t GetUnicodeLength() {
        return GetUnicodeLength2<0, 0>();
    }
};

template<class CharType, size_t len, const CharType (&str)[len]>
struct CompileTimeUtf8ToUnicode {
    static constexpr size_t length = CompileTimeUtf8ToUnicodeLength<CharType, len, str>::GetUnicodeLength();
    CharArray<length * 2> char_array;

    constexpr CompileTimeUtf8ToUnicode() {
        size_t pos = 0;
        size_t wpos = 0;

        while (pos < len - 1) {
            uint32_t code = 0;
            auto c = str[pos];

            if ((c & 0x80) == 0) {
                // 1-byte UTF-8 character (ASCII)
                code = c;
                pos += 1;
            } else if ((c & 0xE0) == 0xC0) {
                // 2-byte UTF-8 character
                code = ((c & 0x1F) << 6) | (static_cast<unsigned char>(str[pos + 1]) & 0x3F);
                pos += 2;
            } else if ((c & 0xF0) == 0xE0) {
                // 3-byte UTF-8 character
                code = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(str[pos + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(str[pos + 2]) & 0x3F);
                pos += 3;
            } else if ((c & 0xF8) == 0xF0) {
                // 4-byte UTF-8 character
                code = ((c & 0x07) << 18) | ((static_cast<unsigned char>(str[pos + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(str[pos + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(str[pos + 3]) & 0x3F);
                pos += 4;
            } else {
                // Invalid UTF-8 start byte
                throw "not utf8";
            }

            char_array[wpos * 2] = code & 0xff;
            char_array[wpos * 2 + 1] = code >> 8;
            ++wpos;
        }

        if (pos != len - 1) {
            throw "not utf8";
        }
        
    }
};

static constexpr char8_t str[] = u8"1我的世界234";
static constexpr auto test = CompileTimeUtf8ToUnicode<char8_t, sizeof(str), str>{}.char_array.desc_len;
