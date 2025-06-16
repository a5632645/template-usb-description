#include "comp.hpp"
#include "tpusb/usb_str.hpp"

/* Manufacturer Descriptor */
constexpr uint8_t  MyManuInfo[] =
{
    0x0E, 0x03, 'w', 0, 'c', 0, 'h', 0, '.', 0, 'c', 0, 'n', 0
};

/* Product Information */
constexpr uint8_t  MyProdInfo[] =
{
    0x12, 0x03, 'C', 0, 'H', 0, '3', 0, '2', 0, 'V', 0, '3', 0, '0', 0, 'x', 0
};

/* Serial Number Information */
constexpr uint8_t  MySerNumInfo[] =
{
    0x16, 0x03, '0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0
              , '6', 0, '7', 0, '8', 0, '9', 0
};

static constexpr auto str1 = USB_STR(u8"wch.cn");
static constexpr auto str2 = USB_STR(u8"CH32V30x");
static constexpr auto str3 = USB_STR(u8"0123456789");

static constexpr auto cmp1 = Compare(MyManuInfo, str1.char_array.desc);
static constexpr auto cmp2 = Compare(MyProdInfo, str2.char_array.desc);
static constexpr auto cmp3 = Compare(MySerNumInfo, str3.char_array.desc);
