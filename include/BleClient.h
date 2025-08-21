#pragma once
#include "Types.h"
#include "Errors.h"
#include "Events.h"
#include "Futures.h"
#include "Adapter.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace ble {

struct TransactionId { std::string value; };

class BleClient {
public:
  static std::unique_ptr<BleClient> create(AdapterPtr adapter);

  virtual ~BleClient() = default;

  // ---- Events ----
  virtual ListenerId onEvent(std::function<void(const Event&)> cb) = 0;
  virtual void removeListener(ListenerId id) = 0;

  virtual AdapterState adapterState() const = 0;

  // ---- Scanning ----
  virtual std::error_code startScan(const ScanOptions& opts,
                                    std::optional<TransactionId> tx = std::nullopt) = 0;
  virtual void stopScan() = 0;

  // ---- Connections ----
  virtual FutureVoid connect(const DeviceId& id, const ConnectOptions& opts = {},
                             std::optional<TransactionId> tx = std::nullopt) = 0;
  virtual FutureVoid disconnect(const DeviceId& id) = 0;

  // ---- Discovery ----
  virtual Future<std::vector<Service>> discoverServices(const DeviceId& id) = 0;
  virtual Future<std::vector<Characteristic>> discoverCharacteristics(const DeviceId& id, const Uuid& service) = 0;
  virtual Future<std::vector<Descriptor>> discoverDescriptors(const DeviceId& id, const Uuid& service,
                                                              const Uuid& chr) = 0;

  // ---- GATT ops ----
  virtual Future<ByteArray> read(const DeviceId& id, const Uuid& svc, const Uuid& chr,
                                 std::optional<TransactionId> tx = std::nullopt) = 0;
  virtual FutureVoid write(const DeviceId& id, const Uuid& svc, const Uuid& chr, const ByteArray& data,
                           const WriteOptions& opts = {}, std::optional<TransactionId> tx = std::nullopt) = 0;

  // ---- Notifications ----
  virtual FutureVoid subscribe(const DeviceId& id, const Uuid& svc, const Uuid& chr) = 0;
  virtual FutureVoid unsubscribe(const DeviceId& id, const Uuid& svc, const Uuid& chr) = 0;

  // ---- Extras ----
  virtual Future<int> readRssi(const DeviceId& id) = 0;
  virtual FutureVoid requestMtu(const DeviceId& id, int mtu) = 0;
  virtual std::error_code setConnectionPriority(const DeviceId& id, PriorityOptions opts) = 0;

  // ---- Transactions ----
  virtual bool cancel(const TransactionId& tx) = 0;

  // ---- Cache/Registry ----
  virtual std::optional<Device> device(const DeviceId& id) const = 0;
  virtual std::vector<Device> knownDevices() const = 0;
};

} // namespace ble
