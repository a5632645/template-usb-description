#pragma once
#include "usb.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

// --------------------------------------------------------------------------------
// USB MIDI 1.0  https://usb.org/sites/default/files/midi10.pdf
// TODO
//     check compacable with uac
// --------------------------------------------------------------------------------

namespace tpusb::midiv1 {

/**
 * @brief USB MIDI 1.0 Jack 描述符的标记基类。
 *
 * MIDIAdapterHeader 使用该基类识别并统计 MIDI Jack/Element 描述符。
 */
struct IMidiJackDesc {};

/**
 * @brief USB AudioControl接口初始化参数，用于承载MIDIStreaming接口集合。
 */
struct AudioControlInitPack {
    uint8_t interface_no;
    uint8_t str_id;
    uint16_t bcd_adc;
};

/**
 * @brief USB Audio Class 1.0 AudioControl接口。
 *
 * @tparam NUM_STREAMING_INTERFACES 关联的MIDIStreaming接口数量
 */
template<size_t NUM_STREAMING_INTERFACES>
struct AudioControlInterface : public IInterface {
    static constexpr size_t cs_header_len = 8 + NUM_STREAMING_INTERFACES;
    static constexpr size_t len = 9 + cs_header_len;
    CharArray<len> char_array {
        9,
        4,
    };

    /**
     * @brief 构造 USB AudioControl 接口描述符。
     *
     * @param[in] pack AudioControl 接口的初始化参数
     * @param[in] streaming_interfaces 关联的 MIDIStreaming 接口编号数组
     */
    constexpr AudioControlInterface(
        AudioControlInitPack pack,
        std::array<uint8_t, NUM_STREAMING_INTERFACES> streaming_interfaces
    ) {
        char_array[2] = pack.interface_no;
        char_array[3] = 0;
        char_array[4] = 0;
        char_array[5] = 1;
        char_array[6] = 1;
        char_array[7] = 0;
        char_array[8] = pack.str_id;

        char_array[9] = cs_header_len;
        char_array[10] = 0x24;
        char_array[11] = 1;
        char_array[12] = pack.bcd_adc & 0xff;
        char_array[13] = pack.bcd_adc >> 8;
        char_array[14] = cs_header_len & 0xff;
        char_array[15] = cs_header_len >> 8;
        char_array[16] = NUM_STREAMING_INTERFACES;

        for (size_t i = 0; i < NUM_STREAMING_INTERFACES; ++i) {
            char_array[17 + i] = streaming_interfaces[i];
        }
    }
};

/**
 * @brief MIDI Jack 的类型。
 */
enum class MidiJackType {
    EMBEDDED = 1,
    EXTERNAL = 2,
};

/**
 * @brief MIDI IN Jack 描述符。
 *
 * 该描述符表示 MIDI 数据流进入设备或从设备外部进入的 Jack。
 */
struct MIDIinJack {
    static constexpr size_t len = 6;
    CharArray<len> char_array {
        len,
        0x24,
        2,

    };

    /**
     * @brief 构造 MIDI IN Jack 描述符。
     *
     * @param[in] type Jack 类型
     * @param[in] jack_id Jack 标识符，必须大于 0
     * @param[in] str_id 描述 Jack 名称的字符串描述符索引
     * @exception const char* 当 jack_id 小于 1 时抛出
     */
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

/**
 * @brief MIDI OUT Jack 的输入引脚连接关系。
 * 将source_id的MidiInJack的第pin_connect脚连接到本MidiOutJack
 */
struct MidiOutJackConnection {
    uint8_t source_id;
    uint8_t pin_connect;
};

/**
 * @brief MIDI OUT Jack 描述符。
 *
 * @tparam NUM_INPUT_PINS 该 Jack 的输入引脚数量
 */
template<size_t NUM_INPUT_PINS>
struct MidiOutJack {
    static constexpr size_t len = 7 + 2 * NUM_INPUT_PINS;
    CharArray<len> char_array {
        len,
        0x24,
        3,
    };

    /**
     * @brief 构造 MIDI OUT Jack 描述符。
     *
     * @param[in] type Jack 类型
     * @param[in] jack_id Jack 标识符
     * @param[in] str_id 描述 Jack 名称的字符串描述符索引
     * @param[in] connections 各输入引脚对应的源 Jack 和源引脚
     * @exception const char* 当任意连接的 pin_connect 小于 1 时抛出
     */
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

/**
 * @brief 由 Embedded MIDI IN Jack 和 External MIDI OUT Jack 组成的 Jack 描述符集合。
 */
struct ExternalMidiOutJack : public IMidiJackDesc {
    static constexpr size_t len = MIDIinJack::len + MidiOutJack<1>::len;
    CharArray<len> char_array;

    /**
     * @brief 构造 External MIDI OUT Jack 描述符集合。
     *
     * @param[in] usb_in_id 设备侧 Embedded MIDI IN Jack 的标识符
     * @param[in] out_jack_id 外部 MIDI OUT Jack 的标识符
     * @exception const char* 当 usb_in_id 小于 1 时抛出
     */
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

/**
 * @brief 由 External MIDI IN Jack 和 Embedded MIDI OUT Jack 组成的 Jack 描述符集合。
 */
struct ExternalMidiInJack : public IMidiJackDesc {
    static constexpr size_t len = MIDIinJack::len + MidiOutJack<1>::len;
    CharArray<len> char_array;

    /**
     * @brief 构造 External MIDI IN Jack 描述符集合。
     *
     * @param[in] usb_out_id 设备侧 Embedded MIDI OUT Jack 的标识符
     * @param[in] in_jack_id 外部 MIDI IN Jack 的标识符
     * @exception const char* 当 in_jack_id 小于 1 时抛出
     */
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
            MidiJackType::EMBEDDED,
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

/**
 * @brief MIDI Element 输入 Pin 的连接关系。
 *
 * @note source_id 为源 Entity 的标识符，pin_connect 为该 Entity 的输出 Pin 编号。
 */
struct MidiElementConnection {
    uint8_t source_id;
    uint8_t pin_connect;
};

/**
 * @brief USB MIDI 1.0 Element 描述符。
 *
 * Element 用于描述 MIDI 数据的处理实体，并可连接输入 Terminal、输出 Terminal
 * 以及一个或多个输入 Entity。描述符中的能力位图长度由模板参数决定。
 *
 * @tparam NUM_INPUT_PINS Element 的输入 Pin 数量
 * @tparam NUM_CAPABILITY_BYTES bmElementCaps 的字节数
 * @note 至少应设置一个能力位；没有其他适用能力时建议设置 D7。
 */
template<size_t NUM_INPUT_PINS, size_t NUM_CAPABILITY_BYTES>
struct MidiElement : public IMidiJackDesc {
    static_assert(NUM_INPUT_PINS <= 0xff);
    static_assert(NUM_CAPABILITY_BYTES > 0);
    static_assert(NUM_CAPABILITY_BYTES <= 0xff);

    static constexpr size_t len = 10 + 2 * NUM_INPUT_PINS + NUM_CAPABILITY_BYTES;
    static_assert(len <= 0xff);
    static constexpr uint8_t kDescriptorSubtype = 0x04;
    CharArray<len> char_array {
        len,
        0x24,
        kDescriptorSubtype,
    };

    /**
     * @brief 构造 USB MIDI 1.0 Element 描述符。
     *
     * @param[in] element_id Element 的唯一标识符
     * @param[in] input_pins Element 输入 Pin 的源 Entity 和源 Pin 列表
     * @param[in] num_output_pins Element 的输出 Pin 数量
     * @param[in] in_terminal_link 关联的输入 Terminal 标识符；无关联时为 0
     * @param[in] out_terminal_link 关联的输出 Terminal 标识符；无关联时为 0
     * @param[in] element_caps Element 能力位图
     * @param[in] str_id 描述 Element 的字符串描述符索引
     * @pre element_caps 至少包含 1 个字节
     */
    constexpr MidiElement(
        uint8_t element_id,
        std::array<MidiElementConnection, NUM_INPUT_PINS> input_pins,
        uint8_t num_output_pins,
        uint8_t in_terminal_link,
        uint8_t out_terminal_link,
        std::array<uint8_t, NUM_CAPABILITY_BYTES> element_caps,
        uint8_t str_id
    ) noexcept {
        char_array[3] = element_id;
        char_array[4] = NUM_INPUT_PINS;

        size_t begin = 5;
        for (MidiElementConnection pin : input_pins) {
            char_array[begin++] = pin.source_id;
            char_array[begin++] = pin.pin_connect;
        }

        char_array[begin++] = num_output_pins;
        char_array[begin++] = in_terminal_link;
        char_array[begin++] = out_terminal_link;
        char_array[begin++] = NUM_CAPABILITY_BYTES;

        for (uint8_t capability : element_caps) {
            char_array[begin++] = capability;
        }

        char_array[begin] = str_id;
    }
};

/**
 * @brief USB MIDI 1.0 MIDIStreaming 类头描述符。
 *
 * @tparam DESCS MIDIStreaming 接口中待拼接的描述符类型；统计其中的 MIDI Jack 和 Element 描述符
 */
template<class... DESCS>
struct MIDIAdapterHeader {
    static constexpr size_t kTotalLen = (0 + ... + ((std::is_base_of_v<IMidiJackDesc, DESCS>) ? DESCS::len : 0)) + 7;
    static constexpr size_t len = 7;
    CharArray<7> char_array {
        7,
        0x24,
        1,
        0x00,
        0x01,
        kTotalLen & 0xff,
        kTotalLen >> 8
    };
};

/**
 * @brief MIDI Streaming Class-specific Endpoint 描述符中的 Jack 关联信息。
 *
 * @tparam NUM_MIDI_JACK 关联的 MIDI Jack 数量
 */
template<size_t NUM_MIDI_JACK>
struct EndpointJackAssociation {
    static constexpr size_t len = 4 + NUM_MIDI_JACK;
    CharArray<len> char_array {
        len,
        0x25,
        1,
        NUM_MIDI_JACK
    };

    /**
     * @brief 构造 MIDI Endpoint 的 Jack 关联描述符。
     *
     * @param[in] jacks 与 Endpoint 关联的 Jack 标识符列表
     * @pre jacks 中的标识符数量应与 NUM_MIDI_JACK 一致
     */
    constexpr EndpointJackAssociation(
        std::initializer_list<uint8_t> jacks
    ) {
        size_t begin = 4;
        for (auto jack : jacks) {
            char_array[begin++] = jack;
        }
    }
};

// keep $pack.address and $pack.max_pack_size
/**
 * @brief 带 MIDI Jack 关联信息的 Bulk Endpoint 描述符。
 *
 * @tparam NUM_MIDI_JACK 该 Endpoint 关联的 MIDI Jack 数量
 */
template<size_t NUM_MIDI_JACK>
struct MidiEndpoint : public EndpointLen9<EndpointJackAssociation<NUM_MIDI_JACK>> {
    /**
     * @brief 构造 MIDI Bulk Endpoint 描述符。
     *
     * @param[in] pack Bulk Endpoint 的地址和最大数据包大小
     * @param[in] association Endpoint 关联的 Jack 描述符
     * @note 当前实现将 Endpoint 的 interval、refresh 和 sync_address 固定为 0。
     */
    constexpr MidiEndpoint(
        BulkInitPack pack,
        const EndpointJackAssociation<NUM_MIDI_JACK>& association
    ) : EndpointLen9<EndpointJackAssociation<NUM_MIDI_JACK>>(
        BulkInitPackLen9{
            .address = pack.address,
            .max_pack_size = pack.max_pack_size,
            .interval = 0,
            .refresh = 0,
            .sync_address = 0
        },
        association
    )  {

    }
};

/**
 * @brief USB MIDI 1.0 MIDIStreaming 接口描述符。
 *
 * 该结构会自动添加 MIDIAdapterHeader，并按参数顺序拼接 Jack、Element 和 Endpoint 描述符。
 *
 * @tparam DESCS MIDIStreaming 接口中要拼接的描述符类型
 */
template<class... DESCS>
struct MIDIStreamInterface : public Interface<MIDIAdapterHeader<DESCS...>, DESCS...> {
    /**
     * @brief 构造 USB MIDI 1.0 MIDIStreaming 接口描述符。
     *
     * @param[in] pack MIDIStreaming 接口的初始化参数
     * @param[in] descs 要按顺序拼接到接口中的 Jack、Element 和 Endpoint 描述符
     */
    constexpr MIDIStreamInterface(
        InterfaceInitPackClassed pack,
        const DESCS&... descs)
    : Interface<MIDIAdapterHeader<DESCS...>, DESCS...>(
        InterfaceInitPack{
            .interface_no = pack.interface_no,
            .alter = pack.alter,
            .class_ = 1,
            .subclass = 3,
            .protocol = 0,
            .str_id = pack.str_id
        },
        MIDIAdapterHeader<DESCS...>{},
        descs...
    ) {

    }
};

}
