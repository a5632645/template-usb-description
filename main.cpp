#include <iostream>
#include <format>
#include "constexpr_usb.hpp"
#include "constexpr_uac.hpp"

using namespace usbpp;

// maybe you need this one
// Config<
//     InterfaceAssociation<
//         AudioControlInterface<
//             AudioFunction<
//                 Clock<>,
//                 Inputterminal<>,
//                 OutputTerminal<>,
//                 Feature<>
//             >
//         >,
//         AudioStreamInterface<
//             AudioStreamEndpoint<
//                 Endpoint<>,
//                 AudioStreamEndpointDesc<>
//             >,
//             Endpoint<>
//         >
//     >
// >

static constexpr auto test =
Config{}
.AddInterfaceAssociation(
    InterfaceAssociation{}
    .AddInterface(
        uac::AudioControlInterface{}
        .AddAudioFunction(
            uac::AudioFunction{}
            .AddClock(
                uac::Clock{}
                .SetId(3)
                .SetAttribute(2)
                .SetControls(3)
                .SetAssociateTerminal(0)
                .SetStrId(0)
            )
            .AddInputTerminal(
                uac::InputTerminal{
                    1, 0x0101, 0, 3, 2, 0x3, 0, 0, 0
                }
            )
            .AddOutputTerminal(
                uac::OutputTerminal{}
                .SetId(2)
                .SetTerminalType(0X0301)
                .SetAssociateTerminal(0)
                .SetInputUnitId(4)
                .SetClockId(3)
                .SetControl(0)
                .SetStrId(0)
            )
            .AddFeatureUnit(
                uac::FeatureUnit{}
                .SetUnitId(4)
                .SetSourceId(1)
                .AddFeatrue(0Xf)
                .AddFeatrue(0xf)
                .AddFeatrue(0xf)
                .SetStrId(0)
            )
        )
    )
    .AddInterface(
        uac::AudioStreamInterface{}
        .AddTerminalLink(
            uac::TerminalLink{}
            .SetTerminalLink(1)
            .SetControl(0)
            .SetFormat(1)
            .SetFormats(1)
            .SetNumChannels(2)
            .SetChannelConfig(3)
            .SetChannelNameStrId(0)
        )
        .AddAudioStreamPoint(
            uac::AudioStreamEndpoint{
                IOType::Output, 1, iso, SyncType::Iso, SyncEndpointType::Data, 1, 1024
            }
            .AddEndpointDesc(
                uac::AudioStreamEndpointDesc{}
                .SetAttribute(0)
                .SetControls(0)
                .SetLockDelayUnit(0)
                .SetLockDelay(0)
            )
        )
        .AddEndpoint(
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