#include "tpusb/midiv1.hpp"
#include "example/comp.hpp"
#include "tpusb/usb.hpp"
#include <cstdint>

using namespace tpusb;

static constexpr auto config=
Config{
    ConfigInitPack{
        .config_no = 1,
        .str_id = 0,
        .attribute = 0x80,
        .power = 250
    },
    midiv1::MIDIStreamInterface{
        InterfaceInitPackClassed{
            .interface_no = 0,
            .alter = 0,
            .protocol = 0,
            .str_id = 0
        },
        midiv1::ExternalMidiInJack{
            2, 1
        },
        midiv1::ExternalMidiOutJack{
            3, 4
        },
        midiv1::MidiEndpoint{
            BulkInitPack{
                .address = 0x1, // OUT
                .max_pack_size = 64,
                .interval = 0
            },
            midiv1::EndpointJackAssociation<1>{
                3
            }
        },
        midiv1::MidiEndpoint{
            BulkInitPack{
                .address = 0x81, // IN
                .max_pack_size = 64,
                .interval = 0
            },
            midiv1::EndpointJackAssociation<1>{
                2
            }
        },
    }
};

#define MIDI_IN_PORTS_NUM 1
#define MIDI_OUT_PORTS_NUM 1
#define USB_DESC_TYPE_CONFIGURATION 2
#define USB_DESC_TYPE_INTERFACE 4
#define USB_DESC_TYPE_ENDPOINT 5

#define MIDI_EPIN_ADDR 0x81
#define MIDI_EPIN_SIZE 0x40

#define MIDI_EPOUT_ADDR 0x01
#define MIDI_EPOUT_SIZE 0x40

#define USB_MIDI_CLASS_DESC_SHIFT 18
#define USB_MIDI_DESC_SIZE 7
#define USB_MIDI_REPORT_DESC_SIZE (MIDI_IN_PORTS_NUM * 16 + MIDI_OUT_PORTS_NUM * 16 + 33)
#define USB_MIDI_CONFIG_DESC_SIZE (USB_MIDI_REPORT_DESC_SIZE + USB_MIDI_CLASS_DESC_SHIFT)

#define MIDI_DESCRIPTOR_TYPE 0x21

#define MIDI_REQ_SET_PROTOCOL 0x0B
#define MIDI_REQ_GET_PROTOCOL 0x03

#define MIDI_REQ_SET_IDLE 0x0A
#define MIDI_REQ_GET_IDLE 0x02

#define MIDI_REQ_SET_REPORT 0x09
#define MIDI_REQ_GET_REPORT 0x01

#define MIDI_JACK_1 0x01
#define MIDI_JACK_2 0x02
#define MIDI_JACK_3 0x03
#define MIDI_JACK_4 0x04
#define MIDI_JACK_5 0x05
#define MIDI_JACK_6 0x06
#define MIDI_JACK_7 0x07
#define MIDI_JACK_8 0x08
#define MIDI_JACK_9 0x09
#define MIDI_JACK_10 0x0a
#define MIDI_JACK_11 0x0b
#define MIDI_JACK_12 0x0c
#define MIDI_JACK_13 0x0d
#define MIDI_JACK_14 0x0e
#define MIDI_JACK_15 0x0f
#define MIDI_JACK_16 0x10
#define MIDI_JACK_17 (MIDI_IN_PORTS_NUM * 2 + 0x01)
#define MIDI_JACK_18 (MIDI_IN_PORTS_NUM * 2 + 0x02)
#define MIDI_JACK_19 (MIDI_IN_PORTS_NUM * 2 + 0x03)
#define MIDI_JACK_20 (MIDI_IN_PORTS_NUM * 2 + 0x04)
#define MIDI_JACK_21 (MIDI_IN_PORTS_NUM * 2 + 0x05)
#define MIDI_JACK_22 (MIDI_IN_PORTS_NUM * 2 + 0x06)
#define MIDI_JACK_23 (MIDI_IN_PORTS_NUM * 2 + 0x07)
#define MIDI_JACK_24 (MIDI_IN_PORTS_NUM * 2 + 0x08)
#define MIDI_JACK_25 (MIDI_IN_PORTS_NUM * 2 + 0x09)
#define MIDI_JACK_26 (MIDI_IN_PORTS_NUM * 2 + 0x0a)
#define MIDI_JACK_27 (MIDI_IN_PORTS_NUM * 2 + 0x0b)
#define MIDI_JACK_28 (MIDI_IN_PORTS_NUM * 2 + 0x0c)
#define MIDI_JACK_29 (MIDI_IN_PORTS_NUM * 2 + 0x0d)
#define MIDI_JACK_30 (MIDI_IN_PORTS_NUM * 2 + 0x0e)
#define MIDI_JACK_31 (MIDI_IN_PORTS_NUM * 2 + 0x0f)
#define MIDI_JACK_32 (MIDI_IN_PORTS_NUM * 2 + 0x10)
/* USB MIDI device Configuration Descriptor */
static constexpr uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZE] =
{
        0x09,                        /* bLength: Configuration Descriptor size */
        USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
        USB_MIDI_CONFIG_DESC_SIZE,
        0x00, /*Length of the total configuration block, including this descriptor, in bytes.*/
        0x01, /*bNumInterfaces: 1 interface*/
        0x01, /*bConfigurationValue: ID of this configuration. */
        0x00, /*iConfiguration: Index of string descriptor describing the configuration (Unused.)*/
        0x80, /*bmAttributes: Bus Powered device, not Self Powered, no Remote wakeup capability. */
        0xFA, /*MaxPower 500 mA: this current is used for detecting Vbus*/

        /************** MIDI Adapter Standard MS Interface Descriptor ****************/
        0x09,                    /*bLength: Interface Descriptor size*/
        USB_DESC_TYPE_INTERFACE, /*bDescriptorType: Interface descriptor type*/
        0x00,                    /*bInterfaceNumber: Index of this interface.*/
        0x00,                    /*bAlternateSetting: Alternate setting*/
        0x02,                    /*bNumEndpoints*/
        0x01,                    /*bInterfaceClass: AUDIO*/
        0x03,                    /*bInterfaceSubClass : MIDISTREAMING*/
        0x00,                    /*nInterfaceProtocol : Unused*/
        0x00,                    /*iInterface: Unused*/

        /******************** MIDI Adapter Class-specific MS Interface Descriptor ********************/
        /* USB_MIDI_CLASS_DESC_SHIFT */
        0x07, /*bLength: Descriptor size*/
        0x24, /*bDescriptorType: CS_INTERFACE descriptor*/
        0x01, /*bDescriptorSubtype: MS_HEADER subtype*/
        0x00,
        0x01, /*BcdADC: Revision of this class specification*/
        USB_MIDI_REPORT_DESC_SIZE,
        0x00, /*wTotalLength: Total size of class-specific descriptors*/

#if MIDI_IN_PORTS_NUM >= 1
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,        /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,        /*bJackType: EXTERNAL.*/
        MIDI_JACK_1, /*bJackID: ID of this Jack.*/
        0x00,        /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,        /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,        /*bJackType: EMBEDDED*/
        MIDI_JACK_2, /*bJackID: ID of this Jack.*/
        0x01,        /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_1, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,        /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,        /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 2
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,        /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,        /*bJackType: EXTERNAL.*/
        MIDI_JACK_3, /*bJackID: ID of this Jack.*/
        0x00,        /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,        /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,        /*bJackType: EMBEDDED*/
        MIDI_JACK_4, /*bJackID: ID of this Jack.*/
        0x01,        /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_3, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,        /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,        /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 3
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,        /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,        /*bJackType: EXTERNAL.*/
        MIDI_JACK_5, /*bJackID: ID of this Jack.*/
        0x00,        /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,        /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,        /*bJackType: EMBEDDED*/
        MIDI_JACK_6, /*bJackID: ID of this Jack.*/
        0x01,        /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_5, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,        /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,        /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 4
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,        /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,        /*bJackType: EXTERNAL.*/
        MIDI_JACK_7, /*bJackID: ID of this Jack.*/
        0x00,        /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,        /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,        /*bJackType: EMBEDDED*/
        MIDI_JACK_8, /*bJackID: ID of this Jack.*/
        0x01,        /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_7, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,        /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,        /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 5
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,        /*bLength: Size of this descriptor, in bytes*/
        0x24,        /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,        /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,        /*bJackType: EXTERNAL.*/
        MIDI_JACK_9, /*bJackID: ID of this Jack.*/
        0x00,        /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_10, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_9,  /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 6
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_11, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_12, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_11, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 7
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_13, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_14, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_13, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_IN_PORTS_NUM >= 8
        /******************** MIDI Adapter MIDI IN Jack Descriptor (External) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_15, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (Embedded) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_16, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_15, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 1
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_17, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_18, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_17, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 2
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_19, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_20, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_19, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 3
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_21, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_22, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_21, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 4
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_23, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_24, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_23, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 5
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_25, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_26, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_25, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 6
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_27, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_28, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_27, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 7
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_29, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_30, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_29, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

#if MIDI_OUT_PORTS_NUM >= 8
        /******************** MIDI Adapter MIDI IN Jack Descriptor (Embedded) ********************/
        0x06,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x02,         /*bDescriptorSubtype: MIDI_IN_JACK subtype*/
        0x01,         /*bJackType: EMBEDDED*/
        MIDI_JACK_31, /*bJackID: ID of this Jack.*/
        0x00,         /*iJack: Unused.*/

        /******************** MIDI Adapter MIDI OUT Jack Descriptor (External) ********************/
        0x09,         /*bLength: Size of this descriptor, in bytes*/
        0x24,         /*bDescriptorType: CS_INTERFACE descriptor.*/
        0x03,         /*bDescriptorSubtype: MIDI_OUT_JACK subtype*/
        0x02,         /*bJackType: EXTERNAL.*/
        MIDI_JACK_32, /*bJackID: ID of this Jack.*/
        0x01,         /*bNrInputPins: Number of Input Pins of this Jack.*/
        MIDI_JACK_31, /*BaSourceID(1): ID of the Entity to which this Pin is connected.*/
        0x01,         /*BaSourcePin(1): Output Pin number of the Entity to which this Input Pin is connected.*/
        0x00,         /*iJack: Unused.*/
#endif

        /******************** MIDI Adapter Standard Bulk OUT Endpoint Descriptor ********************/
        0x09,                   /*bLength: Size of this descriptor, in bytes*/
        USB_DESC_TYPE_ENDPOINT, /*bDescriptorType: ENDPOINT descriptor.*/
        MIDI_EPOUT_ADDR,        /*bEndpointAddress: OUT Endpoint 1.*/
        0x02,                   /*bmAttributes: Bulk, not shared.*/
        MIDI_EPOUT_SIZE,
        0x00, /*wMaxPacketSize*/
        0x00, /*bInterval: Ignored for Bulk. Set to zero.*/
        0x00, /*bRefresh: Unused.*/
        0x00, /*bSynchAddress: Unused.*/

        /******************** MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor ********************/
        (4 + MIDI_OUT_PORTS_NUM), /*bLength: Size of this descriptor, in bytes*/
        0x25,                     /*bDescriptorType: CS_ENDPOINT descriptor*/
        0x01,                     /*bDescriptorSubtype: MS_GENERAL subtype.*/
        MIDI_OUT_PORTS_NUM,       /*bNumEmbMIDIJack: Number of embedded MIDI IN Jacks.*/
#if MIDI_OUT_PORTS_NUM >= 1
        MIDI_JACK_17, /*BaAssocJackID(1): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 2
        MIDI_JACK_19, /*BaAssocJackID(2): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 3
        MIDI_JACK_21, /*BaAssocJackID(3): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 4
        MIDI_JACK_23, /*BaAssocJackID(4): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 5
        MIDI_JACK_25, /*BaAssocJackID(5): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 6
        MIDI_JACK_27, /*BaAssocJackID(6): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 7
        MIDI_JACK_29, /*BaAssocJackID(7): ID of the Embedded MIDI IN Jack.*/
#endif
#if MIDI_OUT_PORTS_NUM >= 8
        MIDI_JACK_31, /*BaAssocJackID(8): ID of the Embedded MIDI IN Jack.*/
#endif

        /******************** MIDI Adapter Standard Bulk IN Endpoint Descriptor ********************/
        0x09,                   /*bLength: Size of this descriptor, in bytes*/
        USB_DESC_TYPE_ENDPOINT, /*bDescriptorType: ENDPOINT descriptor.*/
        MIDI_EPIN_ADDR,         /*bEndpointAddress: IN Endpoint 1.*/
        0x02,                   /*bmAttributes: Bulk, not shared.*/
        MIDI_EPIN_SIZE,
        0x00, /*wMaxPacketSize*/
        0x00, /*bInterval: Ignored for Bulk. Set to zero.*/
        0x00, /*bRefresh: Unused.*/
        0x00, /*bSynchAddress: Unused.*/

        /******************** MIDI Adapter Class-specific Bulk IN Endpoint Descriptor ********************/
        (4 + MIDI_IN_PORTS_NUM), /*bLength: Size of this descriptor, in bytes*/
        0x25,                    /*bDescriptorType: CS_ENDPOINT descriptor*/
        0x01,                    /*bDescriptorSubtype: MS_GENERAL subtype.*/
        MIDI_IN_PORTS_NUM,       /*bNumEmbMIDIJack: Number of embedded MIDI OUT Jacks.*/
#if MIDI_IN_PORTS_NUM >= 1
        MIDI_JACK_2, /*BaAssocJackID(1): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 2
        MIDI_JACK_4, /*BaAssocJackID(2): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 3
        MIDI_JACK_6, /*BaAssocJackID(3): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 4
        MIDI_JACK_8, /*BaAssocJackID(4): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 5
        MIDI_JACK_10, /*BaAssocJackID(5): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 6
        MIDI_JACK_12, /*BaAssocJackID(6): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 7
        MIDI_JACK_14, /*BaAssocJackID(7): ID of the Embedded MIDI OUT Jack.*/
#endif
#if MIDI_IN_PORTS_NUM >= 8
        MIDI_JACK_16, /*BaAssocJackID(8): ID of the Embedded MIDI OUT Jack.*/
#endif
};

constexpr CompareResult cmp = Compare(config.char_array.desc, USBD_MIDI_CfgDesc);
static constexpr uint32_t fs[] {
    cmp.a,
    cmp.b,
    cmp.index,
    cmp.diff
};
static_assert(cmp.diff == 0);
