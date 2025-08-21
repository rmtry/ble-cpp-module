#pragma once
#include "Types.h"
#include "Events.h"
#include <functional>
#include <memory>

namespace ble {

// Minimal platform adapter interface. Implement later per-OS.
struct IPlatformAdapter {
  virtual ~IPlatformAdapter() = default;

  virtual AdapterState adapterState() = 0;
  virtual void setEventSink(std::function<void(const Event&)> sink) = 0;

  virtual std::error_code startScan(const ScanOptions& opts) = 0;
  virtual void stopScan() = 0;

  virtual std::error_code connect(const DeviceId& id, const ConnectOptions& opts) = 0;
  virtual std::error_code disconnect(const DeviceId& id) = 0;

  virtual std::error_code discover(const DeviceId& id) = 0;
  virtual std::error_code read(const DeviceId& id, const Uuid& svc, const Uuid& chr, ByteArray& out) = 0;
  virtual std::error_code write(const DeviceId& id, const Uuid& svc, const Uuid& chr,
                                const ByteArray& data, const WriteOptions& opts) = 0;
  virtual std::error_code setNotify(const DeviceId& id, const Uuid& svc, const Uuid& chr, bool enable) = 0;

  virtual std::error_code readDescriptor(const DeviceId&, const Uuid& svc, const Uuid& chr,
                                         const Uuid& desc, ByteArray& out) = 0;
  virtual std::error_code writeDescriptor(const DeviceId&, const Uuid& svc, const Uuid& chr,
                                          const Uuid& desc, const ByteArray& data) = 0;

  virtual std::error_code readRssi(const DeviceId& id, int& outRssi) = 0;
  virtual std::error_code requestMtu(const DeviceId& id, int mtu) = 0;
  virtual std::error_code setConnectionPriority(const DeviceId& id, PriorityOptions opts) = 0;

  // Optional: bonding
  virtual std::error_code createBond(const DeviceId& id) { (void)id; return {}; }
  virtual std::error_code removeBond(const DeviceId& id) { (void)id; return {}; }
};

using AdapterPtr = std::shared_ptr<IPlatformAdapter>;

} // namespace ble
