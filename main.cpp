#include <iostream>
#include <format>
#include "constexpr_usb.hpp"

using namespace usbpp;

static constexpr auto test =
Config{}.AddInterfaceAssociation(
    InterfaceAssociation{}.AddInterface(
        Interface{}.AddEndpoint(
            Endpoint{
                IOType::Output, 1, iso, SyncType::Adaptive, SyncEndpointType::Data, 1, 1024
            }
        ).AddEndpoint(
            Endpoint{
                IOType::Input, 1, iso, SyncType::None, SyncEndpointType::Feedback, 1, 4
            }
        )
    )
);

int main(void) {
    for (auto c : test.char_array.desc) {
        std::cout << std::format("0x{:02x},", c);
    }
}