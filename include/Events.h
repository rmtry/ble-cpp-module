#pragma once
#include "Types.h"
#include <variant>
#include <string>

namespace ble {

enum class AdapterState { Unknown, Resetting, Unsupported, Unauthorized, PoweredOff, PoweredOn };

struct EAdapterStateChanged { AdapterState state; };
struct EScanResult          { Device device; };
struct EDeviceStateChanged  { DeviceId id; bool connected; };
struct ENotification        { DeviceId id; Uuid service; Uuid characteristic; ByteArray value; };
struct ERssiRead            { DeviceId id; int rssi; };

using Event = std::variant<EAdapterStateChanged, EScanResult, EDeviceStateChanged, ENotification, ERssiRead>;

using ListenerId = uint64_t;

} // namespace ble
