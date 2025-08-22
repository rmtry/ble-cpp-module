// AppleAdapter.mm  (Objective-C++)
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

#include "../include/Errors.h"
#include "../include/Adapter.h"
#include "../include/Events.h"
#include <atomic>
#include <functional>

using namespace ble;

@interface BleCentralDelegate : NSObject <CBCentralManagerDelegate> {
@public
  std::function<void(const ble::Event&)> emit; // C++ callback
}
@property(nonatomic, strong) CBCentralManager *mgr;
@end

@implementation BleCentralDelegate

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
  AdapterState s = AdapterState::Unknown;
  switch (central.state) {
    case CBManagerStatePoweredOn:      s = AdapterState::PoweredOn; break;
    case CBManagerStatePoweredOff:     s = AdapterState::PoweredOff; break;
    case CBManagerStateUnauthorized:   s = AdapterState::Unauthorized; break;
    case CBManagerStateUnsupported:    s = AdapterState::Unsupported; break;
    case CBManagerStateResetting:      s = AdapterState::Resetting; break;
    default: break;
  }
  if (emit) emit(Event{EAdapterStateChanged{s}});
}

- (void)centralManager:(CBCentralManager *)central
 didDiscoverPeripheral:(CBPeripheral *)peripheral
     advertisementData:(NSDictionary<NSString*, id> *)advertisementData
                  RSSI:(NSNumber *)RSSI {

  Device dev;
  dev.id = peripheral.identifier.UUIDString.UTF8String;
  if (peripheral.name) dev.name = std::string(peripheral.name.UTF8String);
  if (RSSI) dev.rssi = RSSI.intValue;

  AdvertisementData ad;
  NSString *localName = advertisementData[CBAdvertisementDataLocalNameKey];
  if (localName) ad.localName = std::string(localName.UTF8String);

  NSArray<CBUUID*> *svcUUIDs = advertisementData[CBAdvertisementDataServiceUUIDsKey];
  if (svcUUIDs) {
    for (CBUUID *u in svcUUIDs) {
      NSString *s = u.UUIDString;
      std::string cs = s.UTF8String;
      ad.serviceUuids.push_back(cs);
      dev.serviceUuids.push_back(cs);
    }
  }
  dev.ad = std::move(ad);

  if (emit) emit(Event{EScanResult{std::move(dev)}});
}
@end

namespace {

struct AppleAdapter : public IPlatformAdapter {
  std::function<void(const Event&)> sink_;
  BleCentralDelegate *delegate_ = nil;
  std::atomic<bool> scanning_{false};

  AppleAdapter() {
    delegate_ = [BleCentralDelegate new];
    delegate_.mgr = [[CBCentralManager alloc] initWithDelegate:delegate_ queue:dispatch_get_main_queue()];
  }

    ~AppleAdapter() override {
    if (delegate_) {
        [delegate_.mgr stopScan];
        delegate_.mgr.delegate = nil;     // detach delegate
        delegate_->emit = nullptr;        // drop C++ callback
        delegate_.mgr = nil;              // release manager
        delegate_ = nil;                  // release delegate
    }
    }

  AdapterState adapterState() override {
    if (!delegate_.mgr) return AdapterState::Unknown;
    switch (delegate_.mgr.state) {
      case CBManagerStatePoweredOn:    return AdapterState::PoweredOn;
      case CBManagerStatePoweredOff:   return AdapterState::PoweredOff;
      case CBManagerStateUnauthorized: return AdapterState::Unauthorized;
      case CBManagerStateUnsupported:  return AdapterState::Unsupported;
      case CBManagerStateResetting:    return AdapterState::Resetting;
      default: return AdapterState::Unknown;
    }
  }

  void setEventSink(std::function<void(const Event&)> sink) override {
    sink_ = std::move(sink);
    if (delegate_) delegate_->emit = [this](const Event& e){ if (sink_) sink_(e); };
  }

  std::error_code startScan(const ScanOptions& opts) override {
    if (delegate_.mgr.state != CBManagerStatePoweredOn) {
        return ble::make_error(ble::Error::NotReady);
    }
    if (scanning_.exchange(true)) return {};
    NSMutableArray<CBUUID*> *filter = nil;
    if (!opts.serviceUuids.empty()) {
      filter = [NSMutableArray new];
      for (auto &s : opts.serviceUuids) {
        [filter addObject:[CBUUID UUIDWithString:[NSString stringWithUTF8String:s.c_str()]]];
      }
    }
    NSDictionary *scanOpts = @{ CBCentralManagerScanOptionAllowDuplicatesKey : @(opts.allowDuplicates) };
    [delegate_.mgr scanForPeripheralsWithServices:filter options:scanOpts];
    NSLog(@"[AppleAdapter] scanForPeripheralsWithServices:%@ opts:%@", filter, scanOpts);
    return {};
  }

  void stopScan() override {
    if (!scanning_.exchange(false)) return;
    [delegate_.mgr stopScan];
  }

  // Stubs for now:
  std::error_code connect(const DeviceId&, const ConnectOptions&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code disconnect(const DeviceId&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code discover(const DeviceId&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code read(const DeviceId&, const Uuid&, const Uuid&, ByteArray&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code write(const DeviceId&, const Uuid&, const Uuid&, const ByteArray&, const WriteOptions&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code setNotify(const DeviceId&, const Uuid&, const Uuid&, bool) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code readDescriptor(const DeviceId&, const Uuid&, const Uuid&, const Uuid&, ByteArray&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code writeDescriptor(const DeviceId&, const Uuid&, const Uuid&, const Uuid&, const ByteArray&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code readRssi(const DeviceId&, int&) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code requestMtu(const DeviceId&, int) override { return ble::make_error(ble::Error::Unsupported);; }
  std::error_code setConnectionPriority(const DeviceId&, PriorityOptions) override { return {}; }
};

} // namespace

std::shared_ptr<IPlatformAdapter> makeAppleAdapter() {
  return std::make_shared<AppleAdapter>();
}
