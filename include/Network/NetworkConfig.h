#pragma once
#include <cstdint>

namespace NetworkConfig {
    // Protocol versioning
    constexpr uint32_t PROTOCOL_VERSION = 1;

    // Client identification
    constexpr size_t CLIENT_ID_LENGTH = 16;

    // Using a fixed-size array for session ID instead of string
    // 16 bytes is commonly used as it can store a UUID/GUID
    constexpr size_t SESSION_ID_LENGTH = 16;
}