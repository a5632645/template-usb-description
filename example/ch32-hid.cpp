#include "hid.hpp"
#include "comp.hpp"
#include "usb.hpp"

static constexpr auto hid =
Config{
    ConfigInitPack{
        .config_no = 1,
        .str_id = 0,
        .attribute = 0x80,
        .power = 0x23
    },
    HID_Interface{
        InterfaceInitPackClassed{
            .interface_no = 0,
            .alter = 0,
            .protocol = 0,
            .str_id = 0
        },
        HID_Descriptor<1>{
            HID_Descriptor_InitPack{
                .bcd_hid = 0x0111,
                .country_code = 0x00
            },
            {
                HID_DescriptorLengthDesc{
                    .type = 0x22,
                    .length = 0x22
                }
            }
        },
        Endpoint{
            InterruptInitPack{
                .address = 1,
                .max_pack_size = 0x200,
                .interval =1 
            }
        },
        Endpoint{
            InterruptInitPack{
                .address = 0x82,
                .max_pack_size = 0x200,
                .interval = 1
            }
        }
    }
};

constexpr uint8_t  MyCfgDescr_HS[ ] =
{
    /* Configuration Descriptor */
    0x09,                           // bLength
    0x02,                           // bDescriptorType
    0x29, 0x00,                     // wTotalLength
    0x01,                           // bNumInterfaces
    0x01,                           // bConfigurationValue
    0x00,                           // iConfiguration (String Index)
    0x80,                           // bmAttributes
    0x23,                           // bMaxPower 70mA

    /* Interface Descriptor */
    0x09,                           // bLength
    0x04,                           // bDescriptorType (Interface)
    0x00,                           // bInterfaceNumber 0
    0x00,                           // bAlternateSetting
    0x02,                           // bNumEndpoints 2
    0x03,                           // bInterfaceClass
    0x00,                           // bInterfaceSubClass
    0x00,                           // bInterfaceProtocol
    0x00,                           // iInterface (String Index)

    /* HID Descriptor */
    0x09,                           // bLength
    0x21,                           // bDescriptorType
    0x11, 0x01,                     // bcdHID
    0x00,                           // bCountryCode
    0x01,                           // bNumDescriptors
    0x22,                           // bDescriptorType
    0x22, 0x00,                     // wDescriptorLength

    /* Endpoint Descriptor */
    0x07,                           // bLength
    0x05,                           // bDescriptorType
    0x01,                           // bEndpointAddress: OUT Endpoint 1
    0x03,                           // bmAttributes
    0x00, 0x02,                     // wMaxPacketSize
    0x01,                           // bInterval: 1mS

    /* Endpoint Descriptor */
    0x07,                           // bLength
    0x05,                           // bDescriptorType
    0x82,                           // bEndpointAddress: IN Endpoint 2
    0x03,                           // bmAttributes
    0x00, 0x02,                     // wMaxPacketSize
    0x01,                           // bInterval: 1mS
};

static constexpr auto cmp = Compare(MyCfgDescr_HS, hid.char_array.desc);
static constexpr uint32_t fs[] {
    cmp.a,
    cmp.b,
    cmp.index,
    cmp.diff
};
static_assert(cmp.diff == 0);
