#pragma once
#include "constexpr_usb.hpp"
#include <cstddef>
#include <cstdint>
#include <stdint.h>

namespace usbpp::uac {

struct Clock {
    static constexpr size_t len = 8;
    CharArray<8> char_array;

    constexpr uint8_t& Id() { return char_array[3]; }
    constexpr uint8_t& Attribute() { return char_array[4]; }
    constexpr uint8_t& Controls() { return char_array[5]; }
    constexpr uint8_t& AssociateTerminal() { return char_array[6]; }
    constexpr uint8_t& StrId() { return char_array[7]; }
    constexpr Clock& SetId(uint8_t id) { Id() = id; return *this; }
    constexpr Clock& SetAttribute(uint8_t attr) { Attribute() = attr; return *this; }
    constexpr Clock& SetControls(uint8_t controls) { Controls() = controls; return *this; }
    constexpr Clock& SetAssociateTerminal(uint8_t terminal) { AssociateTerminal() = terminal; return *this; }
    constexpr Clock& SetStrId(uint8_t str_id) { StrId() = str_id; return *this; }

    constexpr Clock() {
        char_array[0] = 8;
        char_array[1] = 0x24;
        char_array[2] = 0x0a;
    }
};

struct InputTerminal {
    static constexpr size_t len = 17;
    CharArray<17> char_array;

    constexpr InputTerminal& SetId(uint8_t id) {
        char_array[3] = id;
        return *this;
    }
    constexpr InputTerminal& SetTerminalType(uint16_t type) {
        char_array[4] = type & 0xff;
        char_array[5] = type >> 8;
        return *this;
    }
    constexpr InputTerminal& SetAssociateTerminal(uint8_t id) {
        char_array[6] = id;
        return *this;
    }
    constexpr InputTerminal& SetClockId(uint8_t id) {
        char_array[7] = id;
        return *this;
    }
    constexpr InputTerminal& SetNumChannels(uint8_t num) {
        char_array[8] = num;
        return *this;
    }
    constexpr InputTerminal& SetChannelConfig(uint32_t config) {
        char_array[9] = config & 0xff;
        char_array[10] = config >> 8;
        char_array[11] = config >> 16;
        char_array[12] = config >> 24;
        return *this;
    }
    constexpr InputTerminal& SetControl(uint16_t control) {
        char_array[13] = control & 0xff;
        char_array[14] = control >> 8;
        return *this;
    }
    constexpr InputTerminal& SetChannelStrId(uint8_t id) {
        char_array[15] = id;
        return *this;
    }
    constexpr InputTerminal& SetStrId(uint8_t id) {
        char_array[16] = id;
        return *this;
    }

    constexpr InputTerminal() {
        char_array[0] = 17;
        char_array[1] = 0x24;
        char_array[2] = 0x02;
    }
    constexpr InputTerminal(
        uint8_t id, uint16_t type, uint8_t associat, uint8_t clock_id, 
        uint8_t num_channels, uint32_t channel_config,
        uint16_t controls, uint8_t channel_str_id, uint8_t terminal_str_id
    ) : InputTerminal() {
        SetId(id);
        SetTerminalType(type);
        SetAssociateTerminal(associat);
        SetClockId(clock_id);
        SetNumChannels(num_channels);
        SetChannelConfig(channel_config);
        SetControl(controls);
        SetChannelStrId(channel_str_id);
        SetStrId(terminal_str_id);
    }
};

struct OutputTerminal {
    static constexpr size_t len = 12;
    CharArray<12> char_array;

    constexpr OutputTerminal& SetId(uint8_t id) {
        char_array[3] = id;
        return *this;
    }
    constexpr OutputTerminal& SetTerminalType(uint16_t type) {
        char_array[4] = type & 0xff;
        char_array[5] = type >> 8;
        return *this;
    }
    constexpr OutputTerminal& SetAssociateTerminal(uint8_t id) {
        char_array[6] = id;
        return *this;
    }
    constexpr OutputTerminal& SetInputUnitId(uint8_t id) {
        char_array[7] = id;
        return *this;
    }
    constexpr OutputTerminal& SetClockId(uint8_t id) {
        char_array[8] = id;
        return *this;
    }
    constexpr OutputTerminal& SetControl(uint16_t control) {
        char_array[9] = control & 0xff;
        char_array[10] = control >> 8;
        return *this;
    }
    constexpr OutputTerminal& SetStrId(uint8_t id) {
        char_array[11] = id;
        return *this;
    }

    constexpr OutputTerminal() {
        char_array[0] = 12;
        char_array[1] = 0x24;
        char_array[2] = 0x03;
    }
};

template<size_t N>
struct BaseFeatureUnit {
    CharArray<N> char_array;

    constexpr BaseFeatureUnit<N>& SetUnitId(uint8_t id) {
        char_array[3] = id;
        return *this;
    }
    constexpr BaseFeatureUnit<N>& SetSourceId(uint8_t id) {
        char_array[4] = id;
        return *this;
    }
    constexpr BaseFeatureUnit<N>& SetStrId(uint8_t id) {
        char_array[N - 1] = id;
        return *this;
    }
    constexpr BaseFeatureUnit<N + 2> AddFeatrue(uint32_t featrue) {
        BaseFeatureUnit<N + 2> ret;
        ret.char_array.MergeArray(char_array, CharArray<2>{});
        ret.char_array[N - 1] = char_array[N - 1]; // copy str id
        ret.char_array[N - 2] = featrue >> 8; // add feature channel
        ret.char_array[N - 3] = featrue & 0xff;
        return ret;
    }

    constexpr BaseFeatureUnit() {
        char_array[0] = 6;
        char_array[1] = 0x24;
        char_array[2] = 6;
    }
};
using FeatureUnit = BaseFeatureUnit<6>;

template<size_t N>
struct BaseAudioFunction {
    CharArray<N> char_array;

    constexpr BaseAudioFunction<N>& SetBCDADC(uint16_t bcd) {
        char_array[3] = bcd & 0xff;
        char_array[4] = bcd >> 8;
        return *this;
    }
    constexpr uint8_t& Catalog() { return char_array[5]; }
    constexpr BaseAudioFunction<N>& SetTotalLength(uint16_t len) {
        char_array[6] = len & 0xff;
        char_array[7] = len >> 8;
        return *this;
    }
    constexpr uint16_t GetTotalLength() {
        return (char_array[7] << 8) | char_array[6];
    }
    constexpr uint8_t& Controls() { return char_array[8]; }

    constexpr BaseAudioFunction() {
        char_array[0] = 9;
        char_array[1] = 0x24;
        char_array[2] = 1;
    }

    constexpr BaseAudioFunction<N + Clock::len> AddClock(Clock clock) {
        BaseAudioFunction<N + Clock::len> ret;
        ret.char_array.MergeArray(char_array, clock.char_array);
        ret.SetTotalLength(GetTotalLength() + Clock::len);
        return ret;
    }

    constexpr BaseAudioFunction<N + InputTerminal::len> AddInputTerminal(InputTerminal it) {
        BaseAudioFunction<N + InputTerminal::len> ret;
        ret.char_array.MergeArray(char_array, it.char_array);
        ret.SetTotalLength(GetTotalLength() + InputTerminal::len);
        return ret;
    }

    constexpr BaseAudioFunction<N + OutputTerminal::len> AddOutputTerminal(OutputTerminal ot) {
        BaseAudioFunction<N + OutputTerminal::len> ret;
        ret.char_array.MergeArray(char_array, ot.char_array);
        ret.SetTotalLength(GetTotalLength() + OutputTerminal::len);
        return ret;
    }

    template<size_t N2>
    constexpr BaseAudioFunction<N + N2> AddFeatureUnit(BaseFeatureUnit<N2> feature) {
        BaseAudioFunction<N + N2> ret;
        ret.char_array.MergeArray(char_array, feature.char_array);
        ret.SetTotalLength(GetTotalLength() + N2);
        return ret;
    }
};
using AudioFunction = BaseAudioFunction<9>;

template<size_t N>
struct BaseAudioControlInterface : public BaseInterface<N> {
    constexpr BaseAudioControlInterface() {
        this->ClassNO() = 1;
        this->Protocol() = 0x20;
    }

    template<size_t N2>
    constexpr BaseAudioControlInterface<N + N2> AddAudioFunction(BaseAudioFunction<N2> function) {
        BaseAudioControlInterface<N + N2> ret;
        ret.char_array.MergeArray(this->char_array, function.char_array);
        return ret;
    }
};
using AudioControlInterface = BaseAudioControlInterface<Interface::len>;

struct TerminalLink {
    static constexpr size_t len = 16;
    CharArray<len> char_array;

    constexpr TerminalLink& SetTerminalLink(uint8_t id) {
        char_array[3] = id;
        return *this;
    }
    constexpr TerminalLink& SetControl(uint8_t control) {
        char_array[4] = control;
        return *this;
    }
    constexpr TerminalLink& SetFormat(uint8_t format) {
        char_array[5] = format;
        return *this;
    }
    constexpr TerminalLink& SetFormats(uint32_t formats) {
        char_array[6] = formats & 0xff;
        char_array[7] = (formats >> 8) & 0xff;
        char_array[8] = (formats >> 16) & 0xff;
        char_array[9] = (formats >> 24) & 0xff;
        return *this;
    }
    constexpr TerminalLink& SetNumChannels(uint8_t num) {
        char_array[10] = num;
        return *this;
    }
    constexpr TerminalLink& SetChannelConfig(uint32_t config) {
        char_array[11] = config & 0xff;
        char_array[12] = (config >> 8) & 0xff;
        char_array[13] = (config >> 16) & 0xff;
        char_array[14] = (config >> 24) & 0xff;
        return *this;
    }
    constexpr TerminalLink& SetChannelNameStrId(uint8_t id) {
        char_array[15] = id;
        return *this;
    }

    constexpr TerminalLink() {
        char_array[0] = 16;
        char_array[1] = 0x24;
        char_array[2] = 0x01;
    }
};

struct AudioStreamEndpointDesc {
    static constexpr size_t len = 8;
    CharArray<len> char_array;

    constexpr AudioStreamEndpointDesc& SetAttribute(uint8_t att) {
        char_array[3] = att;
        return *this;
    }
    constexpr AudioStreamEndpointDesc& SetControls(uint8_t controls) {
        char_array[4] = controls;
        return *this;
    }
    constexpr AudioStreamEndpointDesc& SetLockDelayUnit(uint8_t delay) {
        char_array[5] = delay;
        return *this;
    }
    constexpr AudioStreamEndpointDesc& SetLockDelay(uint16_t delay) {
        char_array[6] = delay & 0xff;
        char_array[7] = delay >> 8;
        return *this;
    }

    constexpr AudioStreamEndpointDesc() {
        char_array[0] = len;
        char_array[1] = 0x25;
        char_array[2] = 1;
    }
};

template<size_t N>
struct BaseAudioStreamEndpoint : public BaseEndpoint<N> {
    static constexpr size_t len = N;

    // using BaseEndpoint<N>::BaseEndpoint;
    constexpr BaseAudioStreamEndpoint() : BaseEndpoint<N>() {}

    constexpr BaseAudioStreamEndpoint(
        IOType io, uint8_t ep_no,
        ControlEp e, uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint<N>(io, ep_no, e, interval, max_pack_size) {}

    constexpr BaseAudioStreamEndpoint(
        IOType io, uint8_t ep_no,
        IsoEp e, SyncType sync, SyncEndpointType ep,
        uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint<N>(io, ep_no, e, sync, ep, interval, max_pack_size) {}

    constexpr BaseAudioStreamEndpoint(
        IOType io, uint8_t ep_no,
        BulkEp e, uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint<N>(io, ep_no, e, interval, max_pack_size) {}

    constexpr BaseAudioStreamEndpoint(
        IOType io, uint8_t ep_no,
        InterruptEp e, uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint<N>(io, ep_no, e, interval, max_pack_size) {}

    constexpr BaseAudioStreamEndpoint<N + AudioStreamEndpointDesc::len> AddEndpointDesc(AudioStreamEndpointDesc desc) {
        BaseAudioStreamEndpoint<N + AudioStreamEndpointDesc::len> ret;
        ret.char_array.MergeArray(this->char_array, desc.char_array);
        return ret;
    }
};
using AudioStreamEndpoint = BaseAudioStreamEndpoint<Endpoint::len>;

template<size_t N>
struct BaseAudioStreamInterface : public BaseInterface<N> {
    constexpr BaseAudioStreamInterface() {
        this->InterfaceNO() = 1;
        this->ClassNO() = 1;
        this->SubClass() = 2;
        this->Protocol() = 0x20;
    }

    constexpr BaseAudioStreamInterface<N + TerminalLink::len> AddTerminalLink(TerminalLink link) {
        BaseAudioStreamInterface<N + TerminalLink::len> ret;
        ret.char_array.MergeArray(this->char_array, link.char_array);
        return ret;
    }
    template<size_t N2>
    constexpr BaseAudioStreamInterface<N + N2> AddAudioStreamPoint(BaseAudioStreamEndpoint<N2> ep) {
        BaseAudioStreamInterface<N + N2> ret;
        ret.AddEndpoint(ep);
        return ret;
    }
};
using AudioStreamInterface = BaseAudioStreamInterface<Interface::len>;

}