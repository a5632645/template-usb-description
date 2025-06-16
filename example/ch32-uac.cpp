#include <cstddef>
#include <cstdint>
#include "usb.hpp"
#include "uac2.hpp"
#include "cdc.hpp"
#include "comp.hpp"

static constexpr auto test =
Config{
    ConfigInitPack{
        1, 0, 0x80, 250
    },
    UAC2_InterfaceAssociation{
        UAC2_InterfaceAssociation_InitPack{
            .str_id = 0,
            .protocol = 0x20
        },
        AudioControlInterface{
            InterfaceInitPackClassed{
                .interface_no = 0,
                .alter = 0,
                .protocol = 0x20,
                .str_id = 0
            },
            AudioFunction{
                AudioFunctionInitPack{
                    0x0200, 1, 0
                },
                Clock{
                    3, 2, 3, 0, 0
                },
                InputTerminal{
                    1, 0x0101, 0, 3, 0, 0, {2, 0x3, 0}
                },
                FeatureUnit<3>{
                    FeatureUnitInitPack{
                        4, 1, 0
                    },
                    {0xf, 0xf, 0xf}
                },
                OutputTerminal{
                    2, 0x0301, 0, 4, 3, 0, 0
                }
            }
        },
        AudioStreamInterface{
            InterfaceInitPackClassed{
                .interface_no = 1,
                .alter = 0,
                .protocol = 0x20,
                .str_id = 0
            },
            TerminalLink{
                1, 0, 1, 1, ChannelInitPack{
                    2, 3, 0
                }
            },
            AudioStreamFormat{
                1, 4, 32
            },
            Endpoint{
                IsochronousInitPack{
                    1, 1024, 1, SynchronousType::Isochronous, IsoEpType::Data
                },
                CustomDesc{
                    std::array{8, 0x25, 0x01, 0, 0, 0, 0, 0}
                }
            },
            Endpoint{
                IsochronousInitPack{
                    0x81, 4, 1, SynchronousType::None, IsoEpType::Feedback
                }
            }
        }
    },
    InterfaceAssociation{
        InterfaceAssociationInitPack{
            2, 2, 1, 0
        },
        CDCControlInterface{
            InterfaceInitPackClassed{
                2, 0, 1, 0
            },
            FunctionDesc{
                0x0110
            },
            CDCLength{
                0, 3
            },
            CDCManagement{
                2
            },
            CDCInterfaceSpecify{
                2, 3
            },
            Endpoint{
                InterruptInitPack{
                    0x83, 64, 4
                }
            }
        },
        CDCDataInterface{
            InterfaceInitPackClassed{
                3, 0, 0, 0
            },
            Endpoint{
                BulkInitPack{
                    0x02, 64, 4
                }
            },
            Endpoint{
                BulkInitPack{
                    0x82, 64, 4
                }
            }
        }
    },
};

extern "C" {
const uint8_t* usb_descriptor = test.char_array.desc;
const size_t len = test.char_array.desc_len;
}

#define USB_WORD(X) X & 0xff, X >> 8
#define USB_DWORD(X) X & 0xff, (X >> 8) & 0xff, (X >> 16) & 0xff, (X >> 24)

constexpr uint8_t MyCfgDescr_HS[] =
{
    0x09,           // bLength
    0x02,           // bDescriptorType (Configuration)
    USB_WORD(218),  // wTotalLength
    0x04,           // bNumInterfaces
    0x01,           // bConfigurationValue
    0x00,           // iConfiguration (String Index)
    0x80,           // bmAttributes
    250,            // bMaxPower 500mA

    // interface association descriptor
    8,              // length
    0x0b,           // desc type (INTERFACE_ASSOCIATION)
    0x00,           // first interface
    0x02,           // interface count
    0x01,           // function class (AUDIO)
    0x00,           // function sub class (UNDEFIEND)
    0x20,           // function protocool (AF_VER_2)
    0x00,           // function string id

    // Audio Control Interface Descriptor (Audio Control Interface)
    0x09,           // bLength
    0x04,           // bDescriptorType (Interface)
    0x00,           // bInterfaceNumber
    0x00,           // bAlternateSetting
    0x00,           // bNumEndpoints
    0x01,           // bInterfaceClass (Audio)
    0x01,           // bInterfaceSubClass (Audio Control)
    0x20,           // bInterfaceProtocol
    0x00,           // iInterface

    // ---------- start of audio function ----------
    // Audio Control Interface Header Descriptor
    0x09,           // bLength
    0x24,           // bDescriptorType (CS Interface)
    0x01,           // bDescriptorSubtype (Header)
    0x00, 0x02,     // bcdADC (Audio Device Class Specification Version)
    0x01,           // bCatalog
    USB_WORD(64),   // wTotalLength (Length of this descriptor plus all following descriptors in the control interface)
    0x00,           // controls, all not support

    // Clock Dsec
    8,              // length
    0x24,           // desc type (CS_INTERFACE)
    0x0A,           // desc subtype (CLOCK_SOURCE)
    0x03,           // clock id
    0x02,           // clock attribute (internal variable clock, no sof sync)
    0x03,           // controls (frequency w/r, no valid control)
    0x00,           // associated terminal
    0x00,           // string id

    // Input Terminal
    17,             // length
    0x24,           // desc type (CS_INTERFACE)
    0x02,           // desc subtype (INPUT_TERMINAL)
    0x01,           // terminal id
    0x01, 0x01,     // terminal type (USB streaming)
    0x00,           // associated terminal
    0x03,           // clock source id
    2,           // num channels
    USB_DWORD(0x03),// channel config
    0x00, 0x00,     // control, all not support
    0x00,           // channel string id
    0x00,           // terminal string id

    // feature unit
    18,             // length
    0x24,           // desc type (CS_INTERFACE)
    0x06,           // desc subtype (FEATRUE UNIT)
    0x04,           // unit id
    0x01,           // source id (INPUT TERMINAL)
    USB_DWORD(0xf),   // main (MUTE VOL)
    USB_DWORD(0xf),   // ch0 (MUTE VOL)
    USB_DWORD(0xf),   // ch1 (MUTE VOL)
    0,              // string id

    // Output Terminal
    12,             // length
    0x24,           // desc type (CS_INTERFACE)
    0x03,           // desc subtype (OUTPUT_TERMINAL)
    0x02,           // terminal id
    0x01, 0x03,     // terminal type (speakers)
    0x00,           // associated terminal
    0x04,           // input unit id
    0x03,           // clock source id
    0x00, 0x00,     // controls, all not support
    0x00,           // terminal string id
    // ---------- end of audio function ----------


    // Audio Stream Interface
    // Audio Streaming Interface Descriptor (Alternate Setting 0)
    0x09,  // bLength
    0x04,  // bDescriptorType (Interface)
    0x01,  // bInterfaceNumber (Audio Streaming Interface)
    0x00,  // bAlternateSetting (Alternate Setting 0)
    0x00,  // bNumEndpoints (No endpoints in this setting)
    0x01,  // bInterfaceClass (Audio)
    0x02,  // bInterfaceSubClass (Audio Streaming)
    0x20,  // bInterfaceProtocol
    0x00,  // iInterface (No interface string)

    // Audio Streaming Interface Descriptor (Alternate Setting 1)
    0x09,  // bLength
    0x04,  // bDescriptorType (Interface)
    0x01,  // bInterfaceNumber
    0x01,  // bAlternateSetting (Alternate Setting 1)
    0x02,  // bNumEndpoints (2 endpoint)
    0x01,  // bInterfaceClass (Audio)
    0x02,  // bInterfaceSubClass (Audio Streaming)
    0x20,  // bInterfaceProtocol
    0x00,  // iInterface (No interface string)

    // audio streaming terminal link desc
    16,             // length
    0x24,           // type (CS_INTERFACE)
    0x01,           // subtype (AS_GENERAL)
    0x01,           // terminal link
    0x00,           // no control support
    0x01,           // format type (FORMAT-1)
    USB_DWORD(0x1), // bmFormats
    2,           // num channels
    USB_DWORD(0x3), // channel config
    0x00,           // channel name string id

    // Audio Streaming Format Type Descriptor
    0x06,              // bLength
    0x24,              // bDescriptorType (CS Interface)
    0x02,              // bDescriptorSubtype (Format Type)
    0x01,              // bFormatType (Type I - PCM)
    0x04,              // bSubslotsize
    32,                // bBitResolution

    // Audio Stream Endpoint
    // Audio Streaming Endpoint Descriptor (ISO Data Endpoint)
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x01,        // bEndpointAddress (OUT, EP1)
    0x05,        // bmAttributes (Isochronous, async, data ep)
    USB_WORD(1024),  // wMaxPacketSize (1024 bytes)
    0x01,        // bInterval (2^(X-1) frame)

    // Audio Streaming Endpoint Descriptor (General Audio)
    0x08,        // bLength
    0x25,        // bDescriptorType (CS Endpoint)
    0x01,        // bDescriptorSubtype (General)
    0x00,        // bmAttributes (Sampling Frequency Control)
    0x00,        // controls
    0x00,        // bLockDelayUnits
    0x00, 0x00,  // wLockDelay

    // Audio Streaming Feedback Endpoint Descriptor (ISO Data Endpoint)
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN, EP1)
    0x11,        // bmAttributes (Isochronous, async, explimit feedback)
    USB_WORD(4), // wMaxPacketSize (4 bytes)
    0x01,        // bInterval

    // ---------- usb cdc as debug ----------
    // interface association descriptor
    8,              // length
    0x0b,           // desc type (INTERFACE_ASSOCIATION)
    0x02,           // first interface
    0x02,           // interface count
    0x02,           // function class (CDC)
    0x02,           // function sub class (ACM)
    0x01,           // function protocool (AT commands)
    0x00,           // function string id

    // usb cdc interface desc
    9,           // length
    0x04,        // desc tpye (INTERFACE)
    0x02,        // interface number
    0x00,        // alter settings
    0x01,        // num endpoints
    0x02,        // interface class
    0x02,        // interface sub class
    0x01,        // interface protocol
    0x00,        // interface string id

    /* Functional Descriptors */
    0x05,       // length
    0x24,       // functional desc
    0x00,       // header functional desc
    0x10, 0x01, // cdc version 1.10

    /* Length/management descriptor (data class interface 1) */
    0x05,       // length
    0x24,       // functional desc
    0x01,       // subtype
    0x00,       // capabilities
    0x03,       // data interface

    0x04,       // length
    0x24,       // functional desc
    0x02,       // sub type (CALL MANAGERMANT)
    0x02,       // capabilities

    0x05,       // length
    0x24,       // functional desc
    0x06,       // sub type
    0x02,       // master interface (COMMUICATION)
    0x03,       // slave interface (DATA CLASS)

    /* Interrupt upload endpoint descriptor */
    0x07,       // length
    0x05,       // desc type (ENDPOINT)
    0x83,       // address (EP3 IN)
    0x03,       // attribute
    USB_WORD(64),
    0x04,

    /* Interface 1 (data interface) descriptor */
    0x09,       // length
    0x04,       // desc type (INTERFACE)
    0x03,       // interface num
    0x00,       // alter settings
    0x02,       // num endpoints
    0x0a,       // class
    0x00,       // subclass
    0x00,       // protocol
    0x00,       // string id

    /* Endpoint descriptor */
    0x07,       // length
    0x05,       // desc type (ENDPOINT)
    0x02,       // OUT EP2
    0x02,       // attribute, bluck
    USB_WORD(64), // size
    0x04,       // inverval

    /* Endpoint descriptor */
    0x07,
    0x05,
    0x82,       // IN EP2
    0x02,
    USB_WORD(64),
    0x04,
};

static constexpr auto cmp = Compare(MyCfgDescr_HS, test.char_array.desc);
static constexpr uint32_t fs[] {
    cmp.a,
    cmp.b,
    cmp.index,
    cmp.diff
};
static_assert(cmp.diff == 0);
