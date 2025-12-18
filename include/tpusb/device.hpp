#pragma once
#include "usb.hpp"

// --------------------------------------------------------------------------------
// DEVICE
// TODO
//     add device structures
// --------------------------------------------------------------------------------

namespace tpusb {

struct Device {
    CharArray<18> desc{
        18, // length
        1   // descriptor type
    };

    constexpr Device(
        uint16_t bcd_usb,
        uint8_t device_class,
        uint8_t device_subclass,
        uint8_t device_protocol,
        uint8_t ep0_max_packet_size,
        uint16_t vendor_id,
        uint16_t product_id,
        uint16_t bcd_device,
        uint8_t manufacturer_string_id,
        uint8_t product_string_id,
        uint8_t serial_number_string_id,
        uint8_t num_configurations
    ) {
        desc[2] = bcd_usb & 0xff;
        desc[3] = bcd_usb >> 8;
        desc[4] = device_class;
        desc[5] = device_subclass;
        desc[6] = device_protocol;
        desc[7] = ep0_max_packet_size;
        desc[8] = vendor_id & 0xff;
        desc[9] = vendor_id >> 8;
        desc[10] = product_id & 0xff;
        desc[11] = product_id >> 8;
        desc[12] = bcd_device & 0xff;
        desc[13] = bcd_device >> 8;
        desc[14] = manufacturer_string_id;
        desc[15] = product_string_id;
        desc[16] = serial_number_string_id;
        desc[17] = num_configurations;
    }

    constexpr Device Composite() const {
        Device device = *this;
        device.desc[4] = 0xef;
        device.desc[5] = 0x02;
        device.desc[6] = 0x01;
        return device;
    }
};

}
