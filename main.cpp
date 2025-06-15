#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <type_traits>

#define USBPP_WORD(X) X & 0xff, X >> 8
#define USBPP_DWORD(X) X & 0xff, (X >> 8) & 0xff, (X >> 16) & 0xff, (X >> 24)

template<size_t N>
struct CharArray {
    uint8_t desc[N]{};
    
    constexpr uint8_t& operator[](size_t i) {
        return desc[i];
    }

    constexpr uint8_t operator[](size_t i) const {
        return desc[i];
    }

    template<size_t N2, size_t N3>
    constexpr void MergeArray(const CharArray<N2>& lh, const CharArray<N3>& rh) {
        for (size_t i = 0; i < N2; ++i) {
            desc[i] = lh[i];
        }
        for (size_t i = 0; i < N3; ++i) {
            desc[i + N2] = rh[i];
        }
    }
};

template<class DESC>
static constexpr size_t desc_len = DESC::len;

template<class...DESC>
struct DESC_LEN_SUMMER {
    static constexpr size_t len = (0 + ... + desc_len<DESC>);
};

struct IConfig {};
struct IInterface {};
struct IInterfaceAssociation {};
struct IEndpoint {};

struct BulkInitPack {
    uint8_t address;
    uint16_t max_pack_size;
    uint8_t interval;
};

struct InterruptInitPack {
    uint8_t address;
    uint16_t max_pack_size;
    uint8_t interval;
};

struct IsochronousInitPack {
    uint8_t address;
    uint16_t max_pack_size;
    uint8_t interval;
    uint8_t sync_type;
    uint8_t endpoint_type;
};

struct ControlInitPack {
    uint8_t address;
    uint16_t max_pack_size;
    uint8_t interval;
};

template<class... DESCS>
struct Endpoint : IEndpoint {
    static constexpr size_t len = DESC_LEN_SUMMER<DESCS...>::len + 7;
    CharArray<len> char_array {
        7,
        5,
    };
    size_t begin = 7;

    constexpr Endpoint(BulkInitPack pack, const DESCS&... decs) {
        char_array[2] = pack.address;
        char_array[3] = 0x02;
        char_array[4] = pack.max_pack_size & 0xff;
        char_array[5] = pack.max_pack_size >> 8;
        char_array[6] = pack.interval;
        (AppendDesc(decs),...);
    }

    constexpr Endpoint(InterruptInitPack pack, const DESCS&... decs) {
        char_array[2] = pack.address;
        char_array[3] = 0x03;
        char_array[4] = pack.max_pack_size & 0xff;
        char_array[5] = pack.max_pack_size >> 8;
        char_array[6] = pack.interval;
        size_t i = 7;
        (AppendDesc(i, decs),...);
    }

    constexpr Endpoint(IsochronousInitPack pack, const DESCS&... decs) {
        char_array[2] = pack.address;
        char_array[3] = 0x01 | (pack.sync_type << 2) | (pack.endpoint_type << 4);
        char_array[4] = pack.max_pack_size & 0xff;
        char_array[5] = pack.max_pack_size >> 8;
        char_array[6] = pack.interval;
        size_t i = 7;
        (AppendDesc(i, decs),...);
    }

    constexpr Endpoint(ControlInitPack pack, const DESCS&... decs) {
        char_array[2] = pack.address;
        char_array[3] = 0;
        char_array[4] = pack.max_pack_size & 0xff;
        char_array[5] = pack.max_pack_size >> 8;
        char_array[6] = pack.interval;
        size_t i = 7;
        (AppendDesc(i, decs),...);
    }

    template<class DESC>
    constexpr void AppendDesc(const DESC& desc) {
        size_t desc_len = desc.len;
        for (size_t i = 0; i < desc_len ; ++i) {
            char_array[begin + i] = desc.char_array[i];
        }
        begin += desc_len;
    }
};

struct InterfaceInitPack {
    uint8_t interface_no;
    uint8_t alter;
    uint8_t class_;
    uint8_t subclass;
    uint8_t protocol;
    uint8_t str_id;
};

template<class... DESCS>
struct Interface : IInterface {
    static constexpr size_t len = DESC_LEN_SUMMER<DESCS...>::len + 9;
    CharArray<len> char_array {
        9,
        4,
    };
    size_t begin = 9;

    constexpr Interface(InterfaceInitPack pack, const DESCS&... descs) {
        char_array[2] = pack.interface_no;
        char_array[3] = pack.alter;
        char_array[4] = 0; // num endpoint
        char_array[5] = pack.class_;
        char_array[6] = pack.subclass;
        char_array[7] = pack.protocol;
        char_array[8] = pack.str_id;
        (AppendDesc(descs),...);
    }

    template<class DESC>
    constexpr void AppendDesc(const DESC& desc) {
        if constexpr (std::derived_from<DESC, IEndpoint>) {
            char_array[4]++;
        }

        size_t desc_len = desc.len;
        for (size_t i = 0; i < desc_len ; ++i) {
            char_array[begin + i] = desc.char_array[i];
        }
        begin += desc_len;
    }
};

struct InterfaceAssociationInitPack {
    uint8_t function_class;
    uint8_t function_subclass;
    uint8_t function_protocol;
    uint8_t function_str_id;
};

template<class... DESCS>
struct InterfaceAssociation : IInterfaceAssociation {
    static constexpr size_t len = DESC_LEN_SUMMER<DESCS...>::len + 8;
    CharArray<len> char_array {
        8,
        0xb,
        0, // first interface
        0  // interface count
    };
    size_t begin = 8;

    constexpr InterfaceAssociation(InterfaceAssociationInitPack pack, const DESCS&... desc) {
        char_array[4] = pack.function_class;
        char_array[5] = pack.function_subclass;
        char_array[6] = pack.function_protocol;
        char_array[7] = pack.function_str_id;
        (AppendDesc(desc),...);
    }

    template<class DESC>
    constexpr void AppendDesc(const DESC& desc) {
        if constexpr (std::derived_from<DESC, IInterface>) {
            if (desc.char_array[3] == 0) {
                char_array[3]++;
            }
            if (char_array[3] == 0) {
                char_array[2] = desc.char_array[2];
            }
        }

        size_t desc_len = desc.len;
        for (size_t i = 0; i < desc_len ; ++i) {
            char_array[begin + i] = desc.char_array[i];
        }
        begin += desc_len;
    }
};

struct ConfigInitPack {
    uint8_t config_no;
    uint8_t str_id;
    uint8_t attribute;
    uint8_t power;
};

template<class... DESCS>
struct Config : IConfig {
    static constexpr size_t len = DESC_LEN_SUMMER<DESCS...>::len + 9;
    CharArray<len> char_array {
        9,
        2
    };
    size_t begin = 9;

    constexpr Config(ConfigInitPack pack, const DESCS&... desc) {
        char_array[2] = len & 0xff;
        char_array[3] = len >> 8;
        char_array[4] = 0; // num interface
        char_array[5] = pack.config_no;
        char_array[6] = pack.str_id;
        char_array[7] = pack.attribute;
        char_array[8] = pack.power;
        (AppendDesc(desc),...);
    }

    template<class DESC>
    constexpr void AppendDesc(const DESC& desc) {
        if constexpr (std::derived_from<DESC, IInterface>) {
            if (desc.char_array[3] == 0) {
                char_array[4]++;
            }
        }
        else if constexpr (std::derived_from<DESC, IInterfaceAssociation>) {
            char_array[4] += desc.char_array[3];
        }
        else {
            throw "unkown desc";
        }

        size_t desc_len = desc.len;
        for (size_t i = 0; i < desc_len ; ++i) {
            char_array[begin + i] = desc.char_array[i];
        }
        begin += desc_len;
    }
};

static constexpr auto test =
Config{
    ConfigInitPack{
        0, 0, 0, 0
    },
    Interface{
        InterfaceInitPack{
            0, 0, 0, 0, 0, 0
        }
    },
    Interface{
        InterfaceInitPack{
            0, 1, 0, 0, 0, 0
        },
        Endpoint{
            BulkInitPack{
                0, 0, 0
            }
        }
    }
}.char_array;

// static constexpr auto test =
// Config{}
// .AddInterfaceAssociation(
//     InterfaceAssociation{}
//     .AddInterface(
//         uac::AudioControlInterface{0, 0}
//         .AddAudioFunction(
//             uac::AudioFunction{}
//             .AddClock(
//                 uac::Clock{
//                     3, 2, 3, 0, 0
//                 }
//             )
//             .AddInputTerminal(
//                 uac::InputTerminal{
//                     1, 0x0101, 0, 3, 2, 0x3, 0, 0, 0
//                 }
//             )
//             .AddOutputTerminal(
//                 uac::OutputTerminal{
//                     2, 0x0301, 0, 4, 3, 0, 0
//                 }
//             )
//             .AddFeatureUnit(
//                 uac::FeatureUnit{}
//                 .SetUnitId(4)
//                 .SetSourceId(1)
//                 .AddFeatrue(0Xf)
//                 .AddFeatrue(0xf)
//                 .AddFeatrue(0xf)
//                 .SetStrId(0)
//             )
//         )
//     )
//     .AddInterface(
//         uac::AudioStreamInterface{1, 0}
//     )
//     .AddInterface(
//         uac::AudioStreamInterface{1, 1}
//         .AddTerminalLink(
//             uac::TerminalLink{}
//             .SetTerminalLink(1)
//             .SetControl(0)
//             .SetFormat(1)
//             .SetFormats(1)
//             .SetNumChannels(2)
//             .SetChannelConfig(3)
//             .SetChannelNameStrId(0)
//         )
//         .AddAudioStreamPoint(
//             uac::AudioStreamEndpoint{
//                 IOType::Output, 1, iso, SyncType::Iso, SyncEndpointType::Data, 1, 1024
//             }
//             .AddEndpointDesc(
//                 uac::AudioStreamEndpointDesc{}
//                 .SetAttribute(0)
//                 .SetControls(0)
//                 .SetLockDelayUnit(0)
//                 .SetLockDelay(0)
//             )
//         )
//         .AddEndpoint(
//             Endpoint{
//                 IOType::Input, 1, iso, SyncType::None, SyncEndpointType::Feedback, 1, 4
//             }
//         )
//     )
// );

int main(void) {
    for (auto c : test.desc) {
        std::cout << std::format("0x{:02x},", c);
    }
}