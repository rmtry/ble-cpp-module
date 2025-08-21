#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <array>

namespace ble {

using ByteArray = std::vector<uint8_t>;
using DeviceId  = std::string;

// Simple UUID type (lowercase canonical string, 128-bit normalized)
using Uuid = std::string;

struct AdvertisementData {
  std::optional<std::string> localName;
  std::optional<int> txPowerLevel;
  std::unordered_map<uint16_t, ByteArray> manufacturerData; // companyId -> bytes
  std::unordered_map<Uuid, ByteArray> serviceData;          // uuid -> bytes
  std::vector<Uuid> serviceUuids;
  bool connectable{true};
  std::optional<int> appearance;
  std::optional<int> advertisingFlags;
};

struct Device {
  DeviceId id;
  std::optional<std::string> name;
  std::optional<int> rssi;
  std::vector<Uuid> serviceUuids;
  AdvertisementData ad;
  bool isConnected{false};
  std::optional<bool> isBonded; // Android/Windows only
};

struct Service {
  DeviceId deviceId;
  Uuid uuid;
  bool primary{true};
};

struct Characteristic {
  DeviceId deviceId;
  Uuid serviceUuid;
  Uuid uuid;
  bool isNotifying{false};
  // properties bitset: read/write/notify/indicate/writeNoResp/etc.
  uint32_t properties{0};
};

struct Descriptor {
  DeviceId deviceId;
  Uuid serviceUuid;
  Uuid characteristicUuid;
  Uuid uuid;
};

struct ScanOptions {
  std::vector<Uuid> serviceUuids;
  bool allowDuplicates{false};
  std::optional<bool> scanModeLowLatency; // Android hint
  std::optional<bool> legacy;             // Android hint
};

struct ConnectOptions {
  bool autoConnect{false};                 // Android hint
  std::optional<int> timeoutMs;           // connect timeout
  std::optional<int> requestMtu;          // Android/Windows
  std::optional<bool> refreshGatt;        // Android
};

struct WriteOptions {
  bool withResponse{true}; // iOS/write characteristic type; Android WRITE_TYPE_DEFAULT/NO_RESPONSE
};

struct PriorityOptions { // Android specific; no-ops elsewhere
  enum class Level { Balanced, High, LowPower };
  Level level{Level::Balanced};
};

} // namespace ble
