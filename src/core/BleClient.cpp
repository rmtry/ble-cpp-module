#include "../../include/BleClient.h"
#include "../../include/Errors.h"
#include "../../include/Adapter.h"
#include "../../include/Events.h"
#include <mutex>
#include <memory>
#include <future>
#include <system_error>
#include <vector>
#include <optional>

namespace ble {

// ==== Forward declaration of internal EventBus (implemented in EventBus.cpp) ====
class EventBus {
public:
  EventBus();
  ~EventBus();
  ListenerId add(std::function<void(const Event&)> cb);
  void remove(ListenerId id);
  void emit(const Event& e);
};

// ---------- small helpers for futures ----------
template <typename T>
static Future<T> makeReady(T v) {
  std::promise<T> p;
  p.set_value(std::move(v));
  return p.get_future();
}

static FutureVoid makeReadyVoid() {
  std::promise<void> p;
  p.set_value();
  return p.get_future();
}

template <typename T>
static Future<T> makeError(std::error_code ec) {
  std::promise<T> p;
  p.set_exception(std::make_exception_ptr(std::system_error(ec)));
  return p.get_future();
}

static FutureVoid makeErrorVoid(std::error_code ec) {
  std::promise<void> p;
  p.set_exception(std::make_exception_ptr(std::system_error(ec)));
  return p.get_future();
}

// ================== Implementation ==================
class BleClientImpl : public BleClient {
public:
  explicit BleClientImpl(AdapterPtr adapter)
  : adapter_(std::move(adapter)), bus_(std::make_unique<EventBus>()) {
    // Relay adapter events into our bus
    adapter_->setEventSink([this](const Event& e){ bus_->emit(e); });
  }

  // ---- Events ----
  ListenerId onEvent(std::function<void(const Event&)> cb) override {
    return bus_->add(std::move(cb));
  }
  void removeListener(ListenerId id) override {
    bus_->remove(id);
  }

  AdapterState adapterState() const override {
    return adapter_->adapterState();
  }

  // ---- Scanning ----
  std::error_code startScan(const ScanOptions& opts, std::optional<TransactionId>) override {
    return adapter_->startScan(opts);
  }
  void stopScan() override {
    adapter_->stopScan();
  }

  // ---- Connections ----
  FutureVoid connect(const DeviceId& id, const ConnectOptions& opts,
                     std::optional<TransactionId>) override {
    auto ec = adapter_->connect(id, opts);
    return ec ? makeErrorVoid(ec) : makeReadyVoid();
  }

  FutureVoid disconnect(const DeviceId& id) override {
    auto ec = adapter_->disconnect(id);
    return ec ? makeErrorVoid(ec) : makeReadyVoid();
  }

  // ---- Discovery ----
  Future<std::vector<Service>> discoverServices(const DeviceId& id) override {
    // Until you add a Registry, return Unsupported.
    return makeError<std::vector<Service>>(make_error(Error::Unsupported));
  }

  Future<std::vector<Characteristic>> discoverCharacteristics(const DeviceId& id, const Uuid& svc) override {
    return makeError<std::vector<Characteristic>>(make_error(Error::Unsupported));
  }

  Future<std::vector<Descriptor>> discoverDescriptors(const DeviceId& id, const Uuid& svc,
                                                      const Uuid& chr) override {
    return makeError<std::vector<Descriptor>>(make_error(Error::Unsupported));
  }

  // ---- GATT ops ----
  Future<ByteArray> read(const DeviceId& id, const Uuid& svc, const Uuid& chr,
                         std::optional<TransactionId>) override {
    ByteArray out;
    auto ec = adapter_->read(id, svc, chr, out);
    return ec ? makeError<ByteArray>(ec) : makeReady(std::move(out));
    // Later: queue + async + cancel support.
  }

  FutureVoid write(const DeviceId& id, const Uuid& svc, const Uuid& chr, const ByteArray& data,
                   const WriteOptions& opts, std::optional<TransactionId>) override {
    auto ec = adapter_->write(id, svc, chr, data, opts);
    return ec ? makeErrorVoid(ec) : makeReadyVoid();
  }

  // ---- Notifications ----
  FutureVoid subscribe(const DeviceId& id, const Uuid& svc, const Uuid& chr) override {
    auto ec = adapter_->setNotify(id, svc, chr, true);
    return ec ? makeErrorVoid(ec) : makeReadyVoid();
  }

  FutureVoid unsubscribe(const DeviceId& id, const Uuid& svc, const Uuid& chr) override {
    auto ec = adapter_->setNotify(id, svc, chr, false);
    return ec ? makeErrorVoid(ec) : makeReadyVoid();
  }

  // ---- Extras ----
  Future<int> readRssi(const DeviceId& id) override {
    int rssi = 0;
    auto ec = adapter_->readRssi(id, rssi);
    return ec ? makeError<int>(ec) : makeReady(rssi);
  }

  FutureVoid requestMtu(const DeviceId& id, int mtu) override {
    auto ec = adapter_->requestMtu(id, mtu);
    return ec ? makeErrorVoid(ec) : makeReadyVoid();
  }

  std::error_code setConnectionPriority(const DeviceId& id, PriorityOptions opts) override {
    return adapter_->setConnectionPriority(id, opts);
  }

  // ---- Transactions ----
  bool cancel(const TransactionId&) override {
    // Transaction routing/queueing not implemented yet.
    return false;
  }

  // ---- Cache/Registry ----
  std::optional<Device> device(const DeviceId& id) const override {
    // Registry not implemented yet.
    (void)id;
    return std::nullopt;
  }

  std::vector<Device> knownDevices() const override {
    return {};
  }

private:
  AdapterPtr adapter_;
  std::unique_ptr<EventBus> bus_;
};

// Factory
std::unique_ptr<BleClient> BleClient::create(AdapterPtr adapter) {
  return std::make_unique<BleClientImpl>(std::move(adapter));
}

} // namespace ble
