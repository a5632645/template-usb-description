#pragma once
#include <cstddef>
#include <cstdint>

namespace usbpp {

template<std::size_t N>
struct CharArray {
    uint8_t desc[N]{};
    
    constexpr uint8_t& operator[](std::size_t i) {
        return desc[i];
    }

    template<size_t N2, size_t N3>
    constexpr void MergeArray(CharArray<N2> lh, CharArray<N3> rh) {
        for (size_t i = 0; i < N2; ++i) {
            desc[i] = lh[i];
        }
        for (size_t i = 0; i < N3; ++i) {
            desc[i + N2] = rh[i];
        }
    }
};

enum class IOType {
    Input,
    Output
};

enum class SyncType {
    None,
    Iso,
    Adaptive,
    Sync
};

enum SyncEndpointType {
    Data,
    Feedback,
    ImplicitFeedback
};

static constexpr struct ControlEp {} control;
static constexpr struct BulkEp {} bulk;
static constexpr struct InterruptEp {} interrupt;
static constexpr struct IsoEp{} iso;

template<class T>
struct DescType {
    static constexpr uint8_t type = T::type;
};
template<class T>
static constexpr uint8_t desc_type = DescType<T>::type;

template<size_t N>
struct BaseEndpoint {
    static constexpr uint8_t len = 7;
    static constexpr uint8_t type = 5;

    CharArray<N> char_array;

    constexpr BaseEndpoint() {
        char_array[0] = len;
        char_array[1] = type;
    }

    constexpr BaseEndpoint(
        IOType io, uint8_t ep_no,
        ControlEp, uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint() {
        if (io == IOType::Input) {
            Address() = ep_no | 0x80;
        }
        else {
            Address() = ep_no & 0x7f;
        }
        Attribute() = 0;
        Interval() = interval;
        SetMaxPackageSize(max_pack_size);
    }

    constexpr BaseEndpoint(
        IOType io, uint8_t ep_no,
        IsoEp, SyncType sync, SyncEndpointType ep,
        uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint() {
        if (io == IOType::Input) {
            Address() = ep_no | 0x80;
        }
        else {
            Address() = ep_no & 0x7f;
        }
        uint8_t sync_ = static_cast<uint8_t>(sync);
        uint8_t ep_ = static_cast<uint8_t>(ep);
        Attribute() = 1 | (sync_ << 2) | (ep_ << 4);
        Interval() = interval;
        SetMaxPackageSize(max_pack_size);
    }

    constexpr BaseEndpoint(
        IOType io, uint8_t ep_no,
        BulkEp, uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint() {
        if (io == IOType::Input) {
            Address() = ep_no | 0x80;
        }
        else {
            Address() = ep_no & 0x7f;
        }
        Attribute() = 2;
        Interval() = interval;
        SetMaxPackageSize(max_pack_size);
    }

    constexpr BaseEndpoint(
        IOType io, uint8_t ep_no,
        InterruptEp, uint8_t interval, uint16_t max_pack_size
    ) : BaseEndpoint() {
        if (io == IOType::Input) {
            Address() = ep_no | 0x80;
        }
        else {
            Address() = ep_no & 0x7f;
        }
        Attribute() = 3;
        Interval() = interval;
        SetMaxPackageSize(max_pack_size);
    }

    constexpr uint8_t& Address() { return char_array[2]; }
    constexpr uint8_t& Attribute() { return char_array[3]; }
    constexpr uint16_t GetMaxPackageSize() {
        return char_array[4] | (char_array[5] << 8);
    }
    constexpr void SetMaxPackageSize(uint16_t size) {
        char_array[5] = size >> 8;
        char_array[4] = size & 0xff;
    }
    constexpr uint8_t& Interval() { return char_array[6]; }
};
using Endpoint = BaseEndpoint<7>;

template<size_t N>
struct BaseInterface {
    static constexpr uint8_t len = 9;
    static constexpr uint8_t type = 4;

    CharArray<N> char_array;

    constexpr uint8_t& InterfaceNO() { return char_array[2]; }
    constexpr uint8_t& AlterSetting() { return char_array[3]; }
    constexpr uint8_t& NumEndpoints() { return char_array[4]; }
    constexpr uint8_t& ClassNO() { return  char_array[5]; }
    constexpr uint8_t& SubClass() { return char_array[6]; }
    constexpr uint8_t& Protocol() { return char_array[7]; }
    constexpr uint8_t& StrId() { return char_array[8]; }

    constexpr BaseInterface() {
        char_array[0] = len;
        char_array[1] = type;
    }

    template<size_t N2>
    constexpr BaseInterface<N + N2> AddEndpoint(BaseEndpoint<N2> ep) {
        NumEndpoints() = NumEndpoints() + 1;

        BaseInterface<N + N2> ret;
        ret.char_array.MergeArray(char_array, ep.char_array);
        return ret;
    }

    template<size_t N2>
    constexpr BaseInterface<N + N2> AddOtherDesc(CharArray<N2> desc) {
        BaseInterface<N + N2> ret;
        ret.char_array.MergeArray(char_array, desc);
        return ret;
    }
};
using Interface = BaseInterface<9>;

template<size_t N>
struct BaseInterfaceAssociation {
    static constexpr uint8_t len = 8;
    static constexpr uint8_t type = 11;

    CharArray<N> char_array;

    constexpr uint8_t& FirstInterface() { return char_array[2]; }
    constexpr uint8_t& InterfaceCount() { return char_array[3]; }
    constexpr uint8_t& FunctionClass() { return char_array[4]; }
    constexpr uint8_t& FunctionSubClass() { return char_array[5]; }
    constexpr uint8_t& FunctionProtocol() { return char_array[6]; }
    constexpr uint8_t& FunctionStrId() { return char_array[7]; }
    constexpr BaseInterfaceAssociation& SetFirstInterface(uint8_t first) { FirstInterface() = first; return *this; }
    constexpr BaseInterfaceAssociation& SetInterfaceCount(uint8_t count) { InterfaceCount() = count; return *this; }
    constexpr BaseInterfaceAssociation& SetFunctionClass(uint8_t func_class) { FunctionClass() = func_class; return *this; }
    constexpr BaseInterfaceAssociation& SetFunctionSubClass(uint8_t func_sub_class) { FunctionSubClass() = func_sub_class; return *this; }
    constexpr BaseInterfaceAssociation& SetFunctionProtocol(uint8_t func_protocol) { FunctionProtocol() = func_protocol; return *this; }
    constexpr BaseInterfaceAssociation& SetFunctionStrId(uint8_t func_str_id) { FunctionStrId() = func_str_id; return *this; }

    constexpr BaseInterfaceAssociation() {
        char_array[0] = len;
        char_array[1] = type;
    }

    template<size_t N2>
    constexpr BaseInterfaceAssociation<N + N2> AddInterface(BaseInterface<N2> interface) {
        InterfaceCount() = InterfaceCount() + 1;

        BaseInterfaceAssociation<N + N2> ret;
        ret.char_array.MergeArray(char_array, interface.char_array);
        return ret;
    }
};
using InterfaceAssociation = BaseInterfaceAssociation<8>;

template<size_t N>
struct BaiscConfig {
    static constexpr uint8_t len = 9;
    static constexpr uint8_t type = 2;

    CharArray<N> char_array;

    constexpr void SetTotalLength(uint16_t len) {
        char_array[3] = len >> 8;
        char_array[2] = len & 0xff;
    }
    constexpr uint16_t GetTotalLength() {
        return (char_array[3] << 8) | char_array[2];
    }
    constexpr uint8_t& NumInterface() { return char_array[4]; }
    constexpr uint8_t& ConfigValue() { return char_array[5]; }
    constexpr uint8_t& StrId() { return char_array[6]; }
    constexpr uint8_t& Attribute() { return char_array[7]; }
    constexpr uint8_t& Power() { return char_array[8]; }

    constexpr BaiscConfig() {
        char_array[0] = len;
        char_array[1] = type;
    }

    template<size_t INTERFACE_N>
    constexpr BaiscConfig<N + INTERFACE_N> AddInterface(BaseInterface<INTERFACE_N> interface) {
        NumInterface() = NumInterface() + 1;

        BaiscConfig<N + INTERFACE_N> ret;
        ret.char_array.MergeArray(char_array, interface.char_array);
        ret.SetTotalLength(N + INTERFACE_N);
        return ret;
    }

    template<size_t N2>
    constexpr BaiscConfig<N + N2> AddInterfaceAssociation(BaseInterfaceAssociation<N2> association) {
        association.FirstInterface() = NumInterface();
        NumInterface() = NumInterface() + association.InterfaceCount();

        BaiscConfig<N + N2> ret;
        ret.char_array.MergeArray(char_array, association.char_array);
        ret.SetTotalLength(N + N2);
        return ret;
    }
};
using Config = BaiscConfig<9>;

}
