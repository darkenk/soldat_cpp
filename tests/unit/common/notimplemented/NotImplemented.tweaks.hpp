#pragma once

#include "common/port_utils/SourceLocation.hpp"
#include <array>
#include <string_view>

void TestShowNotImplementedMessage(const std::string_view, const std::string_view,
                                   const source_location &);

namespace PortUtilities::NotImplemented::Config
{
constexpr auto DisabledAreas = std::to_array<std::string_view>({"AUDIO_TEST", "GAMEPLAY_TEST"});
constexpr auto ShowNotImplementedMessage = TestShowNotImplementedMessage;
} // namespace PortUtilities::NotImplemented::Config
