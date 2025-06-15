#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <initializer_list>
#include <iostream>
#include <stdint.h>
#include <sys/stat.h>
#include <type_traits>
#include <utility>
#include <fstream>

#define USB_WORD(X) X & 0xff, X >> 8
#define USB_DWORD(X) X & 0xff, (X >> 8) & 0xff, (X >> 16) & 0xff, (X >> 24)

// --------------------------------------------------------------------------------
// STANDARD USB
// --------------------------------------------------------------------------------
template<size_t N>
struct CharArray {
    static constexpr size_t desc_len = N;
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

enum class SynchronousType {
    None,
    Isochronous,
    Adaptive,
    Synchronous
};

enum class IsoEpType {
    Data,
    Feedback,
    ImplicitFeedback
};

struct IsochronousInitPack {
    uint8_t address;
    uint16_t max_pack_size;
    uint8_t interval;
    SynchronousType sync_type;
    IsoEpType endpoint_type;
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
        (AppendDesc(decs),...);
    }

    constexpr Endpoint(IsochronousInitPack pack, const DESCS&... decs) {
        char_array[2] = pack.address;
        char_array[3] = 0x01 | (static_cast<uint8_t>(pack.sync_type) << 2) | (static_cast<uint8_t>(pack.endpoint_type) << 4);
        char_array[4] = pack.max_pack_size & 0xff;
        char_array[5] = pack.max_pack_size >> 8;
        char_array[6] = pack.interval;
        (AppendDesc(decs),...);
    }

    constexpr Endpoint(ControlInitPack pack, const DESCS&... decs) {
        char_array[2] = pack.address;
        char_array[3] = 0;
        char_array[4] = pack.max_pack_size & 0xff;
        char_array[5] = pack.max_pack_size >> 8;
        char_array[6] = pack.interval;
        (AppendDesc(decs),...);
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

struct IInterface {
    static constexpr size_t interface_no_offset = 2;
    static constexpr size_t alter_offset = 3;
    static constexpr size_t num_endpoint_offset = 4;
    static constexpr size_t class_offset = 5;
    static constexpr size_t subclass_offset = 6;
    static constexpr size_t protocol_offset = 7;
    static constexpr size_t str_id_offset = 8;
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
struct IInterfaceAssociation {
    static constexpr size_t first_interface_offset = 2;
    static constexpr size_t interface_count_offset = 3;
};
template<class T>
struct IInterfaceAssociationCustom {
    constexpr void OnAddToInterfaceAssociation(auto& association) const {
        (void)association;
    }
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
            // TODO: enhance logic
            if (char_array[3] == 0) {
                char_array[2] = desc.char_array[2];
            }
            if (desc.char_array[3] == 0) {
                char_array[3]++;
            }
        }
        else if constexpr (std::derived_from<DESC, IInterfaceAssociationCustom<DESC>>) {
            desc.OnAddToInterfaceAssociation(*this);
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
struct IConfig {
    static constexpr size_t len_offset = 0;
    static constexpr size_t type_offset = 1;
    static constexpr size_t total_len_offset = 2;
    static constexpr size_t num_interface_offset = 4;
    static constexpr size_t str_id_offset = 5;
    static constexpr size_t attribute_offset = 6;
    static constexpr size_t power_offset = 7;
};
template<class T>
struct IConfigCustom {
    constexpr void OnAddToConfig(auto& config) const {
        static_cast<T*>(this)->OnAddToConfig(config);
    }
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
        else if constexpr (std::derived_from<DESC, IConfigCustom<DESC>>) {
            desc.OnAddToConfig(*this);
        }

        size_t desc_len = desc.len;
        for (size_t i = 0; i < desc_len ; ++i) {
            char_array[begin + i] = desc.char_array[i];
        }
        begin += desc_len;
    }
};

// --------------------------------------------------------------------------------
// UAC
// --------------------------------------------------------------------------------
struct AudioFunctionInitPack {
    uint16_t bcd_adc;
    uint8_t catalog;
    uint8_t controls;
};
template<class... DESCS>
struct AudioFunction {
    static constexpr size_t len = DESC_LEN_SUMMER<DESCS...>::len + 9;
    CharArray<len> char_array {
        9,
        0x24,
        0x01
    };
    size_t begin = 9;

    constexpr AudioFunction(AudioFunctionInitPack pack, const DESCS&... desc) {
        char_array[3] = pack.bcd_adc & 0xff;
        char_array[4] = pack.bcd_adc >> 8;
        char_array[5] = pack.catalog;
        char_array[6] = len & 0xff;
        char_array[7] = len >> 8;
        char_array[8] = pack.controls;
        (AppendDesc(desc),...);
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

struct Clock {
    static constexpr size_t len = 8;
    CharArray<len> char_array {
        8,
        0x24,
        0x0a
    };

    constexpr Clock(uint8_t id, uint8_t attribute, uint8_t controls, uint8_t associate, uint8_t str_id) {
        char_array[3] = id;
        char_array[4] = attribute;
        char_array[5] = controls;
        char_array[6] = associate;
        char_array[7] = str_id;
    }
};

struct ChannelInitPack {
    uint8_t num_channel;
    uint32_t channel_config;
    uint8_t channel_str_id;
};

struct InputTerminal {
    static constexpr size_t len = 17;
    CharArray<len> char_array {
        17,
        0x24,
        0x02
    };

    constexpr InputTerminal(uint8_t id, uint16_t type, uint8_t associate, uint8_t clock_id, uint16_t control, uint8_t str_id, ChannelInitPack pack) {
        char_array[3] = id;
        char_array[4] = type & 0xff;
        char_array[5] = type >> 8;
        char_array[6] = associate;
        char_array[7] = clock_id;
        char_array[8] = pack.num_channel;
        char_array[9] = pack.channel_config & 0xff;
        char_array[10] = pack.channel_config >> 8;
        char_array[11] = pack.channel_config >> 16;
        char_array[12] = pack.channel_config >> 24;
        char_array[13] = control & 0xff;
        char_array[14] = control >> 8;
        char_array[15] = pack.channel_str_id;
        char_array[16] = str_id;
    }
};

struct FeatureUnitInitPack {
    uint8_t id;
    uint8_t source_id;
    uint8_t str_id;
};
template<size_t N>
struct FeatureUnit {
    static constexpr size_t len = 6 + N * 4;
    CharArray<len> char_array {
        18,
        0x24,
        0x06
    };

    constexpr FeatureUnit(FeatureUnitInitPack pack, std::array<uint32_t, N> channel_features) {
        char_array[3] = pack.id;
        char_array[4] = pack.source_id;
        char_array[len - 1] = pack.str_id;
        for (size_t i = 0; i < N; ++i) {
            size_t offset = 5 + i * 4;
            uint32_t feature = channel_features[i];
            char_array[offset] = feature & 0xff;
            char_array[offset + 1] = feature >> 8;
            char_array[offset + 2] = feature >> 16;
            char_array[offset + 3] = feature >> 24;
        }
    }
};

struct OutputTerminal {
    static constexpr size_t len = 12;
    CharArray<len> char_array {
        12,
        0x24,
        0x03
    };

    constexpr OutputTerminal(uint8_t id, uint16_t type, uint8_t associate, uint8_t input_id, uint8_t clock_id, uint16_t controls, uint8_t str_id) {
        char_array[3] = id;
        char_array[4] = type & 0xff;
        char_array[5] = type >> 8;
        char_array[6] = associate;
        char_array[7] = input_id;
        char_array[8] = clock_id;
        char_array[9] = controls & 0xff;
        char_array[10] = controls >> 8;
        char_array[11] = str_id;
    }
};

struct AudioControlInterfaceInitPack {
    uint8_t interface_no;
    uint8_t protocol;
    uint8_t str_id;
};
template<class... AUDIOFUNCTION>
struct AudioControlInterface : public Interface<AudioFunction<AUDIOFUNCTION...>> {
    constexpr AudioControlInterface(AudioControlInterfaceInitPack pack, const AudioFunction<AUDIOFUNCTION...>& function) 
    : Interface<AudioFunction<AUDIOFUNCTION...>>(InterfaceInitPack{
        .interface_no = pack.interface_no,
        .alter = 0,
        .class_ = 1,
        .subclass = 1,
        .protocol = pack.protocol,
        .str_id = pack.str_id
    }, function) {}
};

struct AudioStreamFormat {
    static constexpr size_t len = 6;
    CharArray<len> char_array {
        6,
        0x24,
        0x02
    };

    constexpr AudioStreamFormat(uint8_t format_type, uint8_t subslotsize, uint8_t bits) {
        char_array[3] = format_type;
        char_array[4] = subslotsize;
        char_array[5] = bits;
    }
};

struct TerminalLink {
    static constexpr size_t len = 16;
    CharArray<len> char_array {
        len,
        0x24,
        1
    };

    constexpr TerminalLink(uint8_t link, uint8_t control, uint8_t format, uint32_t formats, ChannelInitPack pack) {
        char_array[3] = link;
        char_array[4] = control;
        char_array[5] = format;
        char_array[6] = formats & 0xff;
        char_array[7] = formats >> 8;
        char_array[8] = formats >> 16;
        char_array[9] = formats >> 24;
        char_array[10] = pack.num_channel;
        char_array[11] = pack.channel_config & 0xff;
        char_array[12] = pack.channel_config >> 8;
        char_array[13] = pack.channel_config >> 16;
        char_array[14] = pack.channel_config >> 24;
        char_array[15] = pack.channel_str_id;
    }
};

struct AudioStreamInterfaceInitPack {
    uint8_t interface_no;
    uint8_t protocol;
    uint8_t str_id;
};
template<class... DESCS>
struct AudioStreamInterface : public IConfigCustom<AudioStreamInterface<DESCS...>>, public IInterfaceAssociationCustom<AudioStreamInterface<DESCS...>> {
    static constexpr size_t len = DESC_LEN_SUMMER<DESCS...>::len + TerminalLink::len + 9 * 2;
    CharArray<len> char_array;
    size_t begin = 0;

    // you should add descriptions
    // 1. any @AudioStreamFormat
    // 2. any @Endpoint
    constexpr AudioStreamInterface(
        AudioStreamInterfaceInitPack pack,
        TerminalLink link,
        const DESCS&... desc
    ) {
        Interface<> empty_interface{
            InterfaceInitPack{
                .interface_no = pack.interface_no,
                .alter = 0,
                .class_ = 1,
                .subclass = 2,
                .protocol = pack.protocol,
                .str_id = pack.str_id
            }
        };
        Interface<TerminalLink, DESCS...> interface{
            InterfaceInitPack{
                .interface_no = pack.interface_no,
                .alter = 1,
                .class_ = 1,
                .subclass = 2,
                .protocol = pack.protocol,
                .str_id = pack.str_id
            },
            link,
            desc...
        };
        for (size_t i = 0; i < empty_interface.len; ++i) {
            char_array[i] = empty_interface.char_array[i];
        }
        begin = empty_interface.len;
        for (size_t i = 0; i < interface.len; ++i) {
            char_array[begin + i] = interface.char_array[i];
        }
    }

    constexpr void OnAddToConfig(auto& config) const {
        config.char_array[IConfig::num_interface_offset]++;
    }

    constexpr void OnAddToInterfaceAssociation(auto& association) const {
        if (association.char_array[IInterfaceAssociation::interface_count_offset] == 0) {
            association.char_array[IInterfaceAssociation::first_interface_offset] = char_array[IInterface::interface_no_offset];
        }
        association.char_array[IInterfaceAssociation::interface_count_offset]++;
    }
};

template<size_t N>
struct CustomDesc {
    static constexpr size_t len = N;
    CharArray<len> char_array;

    constexpr CustomDesc(std::array<int, N> init) {
        if (N != init[0]) {
            throw "len not equal";
        }
        for (size_t i = 0; i < N; ++i) {
            char_array[i] = init[i];
        }
    }
};

static constexpr auto test =
Config{
    ConfigInitPack{
        1, 0, 0x80, 250
    },
    InterfaceAssociation{
        InterfaceAssociationInitPack{
            1, 0, 0x20, 0
        },
        AudioControlInterface{
            AudioControlInterfaceInitPack{
                .interface_no = 0,
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
                    }, {0xf, 0xf, 0xf}
                },
                OutputTerminal{
                    2, 0x0301, 0, 4, 3, 0, 0
                }
            }
        },
        // Interface{
        //     InterfaceInitPack{
        //         0, 0, 1, 1, 0x20, 0
        //     },
        //     AudioFunction{
        //         AudioFunctionInitPack{
        //             0x0200, 1, 0
        //         },
        //         Clock{
        //             3, 2, 3, 0, 0
        //         },
        //         InputTerminal{
        //             1, 0x0101, 0, 3, 0, 0, {2, 0x3, 0}
        //         },
        //         FeatureUnit<3>{
        //             FeatureUnitInitPack{
        //                 4, 1, 0
        //             }, {0xf, 0xf, 0xf}
        //         },
        //         OutputTerminal{
        //             2, 0x0301, 0, 4, 3, 0, 0
        //         }
        //     }
        // },

        AudioStreamInterface{
            AudioStreamInterfaceInitPack{
                .interface_no = 1,
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
        // Interface{
        //     InterfaceInitPack{
        //         1, 0, 1, 2, 0x20, 0
        //     }
        // },
        // Interface{
        //     InterfaceInitPack{
        //         1, 1, 1, 2, 0x20, 0
        //     },
        //     CustomDesc{
        //         std::array{16, 0x24, 0x01, 0x01, 0x00, 0x01, 0x01, 0, 0, 0, 2, 0x3, 0, 0, 0, 0}
        //     },
        //     CustomDesc{
        //         std::array{6, 0x24, 0x02, 0x01, 0x04, 32}
        //     },
        //     Endpoint{
        //         IsochronousInitPack{
        //             1, 1024, 1, SynchronousType::Isochronous, IsoEpType::Data
        //         },
        //         CustomDesc{
        //             std::array{8, 0x25, 0x01, 0, 0, 0, 0, 0}
        //         }
        //     },
        //     Endpoint{
        //         IsochronousInitPack{
        //             0x81, 4, 1, SynchronousType::None, IsoEpType::Feedback
        //         }
        //     }
        // }
    },
    InterfaceAssociation{
        InterfaceAssociationInitPack{
            2, 2, 1, 0
        },
        Interface{
            InterfaceInitPack{
                2, 0, 2, 2, 1, 0
            },
            CustomDesc{
                std::array{
                    5,0x24,0,0x10,1
                }
            },
            CustomDesc{
                std::array{
                    5,0x24,1,0,3
                }
            },
            CustomDesc{
                std::array{
                    4,0x24,2,2
                }
            },
            CustomDesc{
                std::array{
                    5,0x24,6,2,3
                }
            },
            Endpoint{
                InterruptInitPack{
                    0x83,64, 4
                }
            }
        },
        Interface{
            InterfaceInitPack{
                3, 0, 0xa, 0, 0, 0
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
    }
}.char_array;

const uint8_t MyCfgDescr_HS[] =
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

int main(void) {
    std::ofstream file{"gen.txt"};
    for (auto c : test.desc) {
        file << std::format("0x{:02x}\n", c);
    }
    std::ofstream file2{"compare.txt"};
    for (auto c : MyCfgDescr_HS) {
        file2 << std::format("0x{:02x}\n", c);
    }
}