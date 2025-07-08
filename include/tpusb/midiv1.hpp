#pragma once
#include "usb.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

// --------------------------------------------------------------------------------
// USB MIDI 1.0  https://usb.org/sites/default/files/midi10.pdf
// TODO
//     test it
//     check compacable with uac
//     add Element
// --------------------------------------------------------------------------------

namespace tpusb{
namespace midiv1{

enum class MidiJackType {
    EMBEDDED = 1,
    EXTERNAL = 2,
};

struct MIDIinJack {
    static constexpr size_t len = 6;
    CharArray<len> char_array {
        len,
        0x24,
        2,

    };

    constexpr MIDIinJack(
        MidiJackType type,
        uint8_t jack_id,
        uint8_t str_id
    ) {
        char_array[3] = static_cast<uint8_t>(type);
        char_array[4] = jack_id;
        char_array[5] = str_id;

        if (jack_id < 1) {
            throw "midi jack id must > 0";
        }
    }
};

struct MidiOutJackConnection {
    uint8_t source_id;
    uint8_t pin_connect;
};

template<size_t NUM_INPUT_PINS>
struct MidiOutJack {
    static constexpr size_t len = 7 + 2 * NUM_INPUT_PINS;
    CharArray<len> char_array {
        len,
        0x24,
        3,
    };

    constexpr MidiOutJack(
        MidiJackType type,
        uint8_t jack_id,
        uint8_t str_id,
        std::array<MidiOutJackConnection, NUM_INPUT_PINS> connections
    ) {
        char_array[3] = static_cast<uint8_t>(type);
        char_array[4] = jack_id;
        char_array[5] = NUM_INPUT_PINS;
        size_t begin = 6;
        for (MidiOutJackConnection c : connections) {
            if (c.pin_connect < 1) {
                throw "midi out jack pin connection must > 0";
            }

            char_array[begin++] = c.source_id;
            char_array[begin++] = c.pin_connect;
        }
        char_array[begin] = str_id;
    }
};

struct ExternalMidiOutJack {
    static constexpr size_t len = MIDIinJack::len + MidiOutJack<1>::len;
    CharArray<len> char_array;

    constexpr ExternalMidiOutJack(
        uint8_t usb_in_id,
        uint8_t out_jack_id
    ) {
        MIDIinJack in_jack{
            MidiJackType::EMBEDDED,
            usb_in_id,
            0
        };
        MidiOutJack<1> out_jack{
            MidiJackType::EXTERNAL,
            out_jack_id,
            0,
            std::array{
                MidiOutJackConnection{
                    .source_id = usb_in_id,
                    .pin_connect = 1,
                }
            }
        };
        size_t off = char_array.Copy(0, in_jack.char_array);
        char_array.Copy(off, out_jack.char_array);
    }
};

struct ExternalMidiInJack {
    static constexpr size_t len = MIDIinJack::len + MidiOutJack<1>::len;
    CharArray<len> char_array;

    constexpr ExternalMidiInJack(
        uint8_t usb_out_id,
        uint8_t in_jack_id
    ) {
        MIDIinJack in_jack{
            MidiJackType::EXTERNAL,
            in_jack_id,
            0
        };
        MidiOutJack<1> out_jack{
            MidiJackType::EXTERNAL,
            usb_out_id,
            0,
            std::array{
                MidiOutJackConnection{
                    .source_id = in_jack_id,
                    .pin_connect = 1,
                }
            }
        };
        size_t off = char_array.Copy(0, in_jack.char_array);
        char_array.Copy(off, out_jack.char_array);
    }
};

// 1. any @ExternalMidiInJack
// 2. any @ExternalMidiOutJack
template<class... JACK_DESCS>
struct MIDIAdapter {
    static constexpr size_t jacks_len = DESC_LEN_SUMMER<JACK_DESCS...>::len;
    static constexpr size_t len = jacks_len + 7;
    CharArray<len> char_array {
        7,
        0x24,
        1,
        0x00,
        0x01,
        jacks_len & 0xff,
        jacks_len >> 8
    };

    constexpr MIDIAdapter(
        const JACK_DESCS&... jacks
    ) {
        (AppendDesc(jacks),...);
    }

    size_t begin = 7;
    template<class DESC>
    constexpr void AppendDesc(const DESC& desc) {
        begin = char_array.Copy(begin, desc.char_array);
    }
};

template<size_t NUM_MIDI_JACK>
struct EndpointJackAssociation {
    static constexpr size_t len = 4 + NUM_MIDI_JACK;
    CharArray<len> char_array {
        len,
        0x25,
        1,
        NUM_MIDI_JACK
    };

    constexpr EndpointJackAssociation(
        std::initializer_list<uint8_t> jacks
    ) {
        size_t begin = 4;
        for (auto jack : jacks) {
            char_array[begin++] = jack;
        }
    }
};

// only keep $pack.address
template<size_t NUM_MIDI_JACK>
struct MidiEndpoint : public EndpointLen9<EndpointJackAssociation<NUM_MIDI_JACK>> {
    constexpr MidiEndpoint(
        BulkInitPack pack,
        const EndpointJackAssociation<NUM_MIDI_JACK>& association
    ) : EndpointLen9<EndpointJackAssociation<NUM_MIDI_JACK>>(
        BulkInitPackLen9{
            .address = pack.address,
            .max_pack_size = 0,
            .interval = 0,
            .refresh = 0,
            .sync_address = 0
        },
        association
    )  {

    }
};

// 1. one @InterfaceInitPackClassed, $protocol is ignore
// 2. one @MIDIAdapter
// 3. any @MidiEndpoint
template<class... DESCS>
struct MIDIStreamInterface : public Interface<DESCS...> {
    constexpr MIDIStreamInterface(
        InterfaceInitPackClassed pack,
        const DESCS&... descs)
    : Interface<DESCS...>(
        InterfaceInitPack{
            .interface_no = pack.interface_no,
            .alter = pack.alter,
            .class_ = 1,
            .subclass = 3,
            .protocol = 0,
            .str_id = pack.str_id
        },
        descs...
    ) {

    }
};

}
}