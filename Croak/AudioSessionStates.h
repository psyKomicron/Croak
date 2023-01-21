#pragma once
#include <stdint.h>

namespace Croak::Audio
{
    enum class AudioSessionStates : uint32_t
    {
        Inactive = 0x1,
        Active = 0x10,
        Expired = 0x100,
        Muted = 0x1000,
        Unmuted = 0x10000,
        DisplayNameChanged = 0x100000,
        IconChanged = 0x1000000,
    };
}