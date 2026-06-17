#include "tpusb/device.hpp"
#include "tpusb/usb.hpp"
#include "tpusb/hid.hpp"
#include "tpusb/uac2.hpp"
#include "tpusb/midiv1.hpp"

enum {
    kMidiJack_UsbIn = 1,
    kMidiJack_ExtIn,
    kMidiJack_UsbOut,
    kMidiJack_ExtOut
};

enum {
    kHidEpAddr_In = 0x81,
    kHidEpMpsize = 64,
    kHidReportSize = 64,
    kHid1EpAddr_In = 0x82,
    kHid1EpAddr_Out = 0x03,
    kHid1EpMpsize = 64,

    kMidiEpAddr_In = 0x84,
    kMidiEpAddr_Out = 0x05,
    kMidiEpMpsize = 64,
};

enum {
    kUsbInterface_HidDebug = 0,
    kUsbInterface_HidMotor,
    kUsbInterface_AudioControl,
    kUsbInterface_MidiStreaming,
    kUsbInterface_Count
};

/*
 * Device Descriptor:
 *   Vendor ID:   0x1A86 (WCH)
 *   Product ID:  0x0002
 *   Class:       0x00 (per-interface)
 */
static constexpr auto kDevice =
tpusb::Device{
    0x0200,     // bcdUSB = 2.00
    0x00,       // bDeviceClass (per interface)
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    64,         // bMaxPacketSize0
    0x1A86,     // idVendor (WCH)
    0x0002,     // idProduct
    0x0005,     // bcdDevice
    1,          // iManufacturer
    2,          // iProduct
    3,          // iSerialNumber
    1           // bNumConfigurations
}.Composite();

/*
 * HID Report Descriptor:
 *   Usage Page 0xFF01 (Vendor Defined)
 *   64-byte Input report (device → host)
 *   Byte 0 = valid data count, bytes 1-63 = payload
 */
static constexpr uint8_t kHidReportDesc[] = {
    0x06, 0x01, 0xFF,  /* Usage Page (Vendor Defined, 0xFF01) */
    0x09, 0x01,        /* Usage (Debug Out) */
    0xA1, 0x01,        /* Collection (Application) */
    0x09, 0x02,        /*   Usage (Debug Data) */
    0x15, 0x00,        /*   Logical Minimum (0) */
    0x25, 0xFF,        /*   Logical Maximum (255) */
    0x75, 0x08,        /*   Report Size (8) */
    0x95, 64, /* Report Count (64) */
    0x81, 0x02,        /*   Input (Data, Var, Abs) */
    0xC0,              /* End Collection */
};

/*
 * HID1 Motor Control Report Descriptor:
 *   Usage Page 0xFF01 (Vendor Defined)
 *   64-byte Output (commands) + 64-byte Input (status)
 */
static constexpr uint8_t kMotorReportDesc[] = {
    0x06, 0x00, 0xFF,       /* Usage Page (Vendor 0xFF01) */
    0x09, 0x03,             /* Usage (Motor Control) */
    0xA1, 0x01,             /* Collection (Application) */
    0x09, 0x04,             /*   Usage (Commands) */
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 64,
    0x91, 0x02,             /*   Output (Data,Var,Abs) */
    0x09, 0x05,             /*   Usage (Status) */
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 64,
    0x81, 0x02,             /*   Input (Data,Var,Abs) */
    0xC0,                   /* End Collection */
};

/*
 * Configuration Descriptor:
 *   Config + HID0(printf) + HID1(motor control) + AudioControl + MIDIStreaming
 *   wTotalLength and bNumInterfaces are auto-calculated by tpusb.
 */
static constexpr auto kConfig =
tpusb::Config{
    tpusb::ConfigInitPack{
        .config_no = 1,
        .str_id = 0,
        .attribute = 0x80,
        .power = 250
    },
    tpusb::hid::HID_Interface{
        tpusb::InterfaceInitPackClassed{
            .interface_no = kUsbInterface_HidDebug,
            .alter = 0,
            .protocol = 0,
            .str_id = 0
        },
        tpusb::hid::HID_Descriptor<1>{
            tpusb::hid::HID_Descriptor_InitPack{
                .bcd_hid = 0x0111,
                .country_code = 0,
            },
            std::array{
                tpusb::hid::HID_DescriptorLengthDesc{
                    .type = 0x22,
                    .length = sizeof(kHidReportDesc)
                }
            }
        },
        tpusb::Endpoint{
            tpusb::InterruptInitPack{
                .address = kHidEpAddr_In,
                .max_pack_size = kHidEpMpsize,
                .interval = 1
            }
        }
    },
    tpusb::hid::HID_Interface{
        tpusb::InterfaceInitPackClassed{
            .interface_no = kUsbInterface_HidMotor,
            .alter = 0,
            .protocol = 0,
            .str_id = 0
        },
        tpusb::hid::HID_Descriptor<1>{
            tpusb::hid::HID_Descriptor_InitPack{
                .bcd_hid = 0x0111,
                .country_code = 0,
            },
            std::array{
                tpusb::hid::HID_DescriptorLengthDesc{
                    .type = 0x22,
                    .length = sizeof(kMotorReportDesc)
                }
            }
        },
        tpusb::Endpoint{
            tpusb::InterruptInitPack{
                .address = kHid1EpAddr_In,
                .max_pack_size = kHid1EpMpsize,
                .interval = 4
            }
        },
        tpusb::Endpoint{
            tpusb::InterruptInitPack{
                .address = kHid1EpAddr_Out,
                .max_pack_size = kHid1EpMpsize,
                .interval = 4
            }
        }
    },
    tpusb::midiv1::AudioControlInterface<1>{
        tpusb::midiv1::AudioControlInitPack{
            .interface_no = kUsbInterface_AudioControl,
            .str_id = 0,
            .bcd_adc = 0x0100
        },
        std::array<uint8_t, 1>{kUsbInterface_MidiStreaming}
    },
    tpusb::midiv1::MIDIStreamInterface{
        tpusb::InterfaceInitPackClassed{
            .interface_no = kUsbInterface_MidiStreaming,
            .alter = 0,
            .protocol = 0,
            .str_id = 0
        },
        tpusb::midiv1::ExternalMidiInJack{kMidiJack_UsbOut, kMidiJack_ExtIn},
        tpusb::midiv1::ExternalMidiOutJack{kMidiJack_UsbIn, kMidiJack_ExtOut},
        tpusb::midiv1::MidiEndpoint<1>{
            tpusb::BulkInitPack{kMidiEpAddr_In, kMidiEpMpsize, 0},
            tpusb::midiv1::EndpointJackAssociation<1>{kMidiJack_UsbOut}
        },
        tpusb::midiv1::MidiEndpoint<1>{
            tpusb::BulkInitPack{kMidiEpAddr_Out, kMidiEpMpsize, 0},
            tpusb::midiv1::EndpointJackAssociation<1>{kMidiJack_UsbIn}
        }
    }
};

// No compare here, because no prototype raw bytes
