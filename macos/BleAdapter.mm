// Build as Objective‑C++ (.mm)
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

#include "../include/Adapter.h"
#include "../include/Events.h"
#include <atomic>
#include <mutex>

using namespace ble;

@interface BleCentralDelegate : NSObject <CBCentralManagerDelegate>
@property(nonatomic, strong) CBCentralManager *mgr;
@property(nonatomic, copy) void (^emit)(const Event &);
@end

@implementation BleCentralDelegate
- (instancetype)initWithEmitter:(void(^)(const Event&))emit {
  if (self = [super init]) { _emit = [emit copy]; }
  return self;
}
- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
  AdapterState s = AdapterState::Unknown;
  switch (central.state) {
    case CBManagerStatePoweredOn:  s = AdapterState::PoweredOn; break;
    case CBManagerStatePoweredOff: s = AdapterState::PoweredOff; break;
    case CBManagerStateUnauthorized: s = AdapterState::Unauthorized; break;
    case CBManagerStateUnsupported: s = AdapterState::Unsupported; break;
    case CBManagerStateResetting: s = AdapterState::Resetting; break;
    default: break;
  }
  if (_emit) _emit(Event{EAdapterStateChanged{s}});
}
- (void)centralManager:(CBCentralManager *)central
 didDiscoverPeripheral:(CBPeripheral *)peripheral
     advertisementData:(NSDictionary<NSString*, id> *)advertisementData
                  RSSI:(NSNumber *)RSSI {

  Device dev;
  dev.id = peripheral.identifier.UUIDString.UTF8String;
  if (peripheral.name) dev.name = std::string(peripheral.name.UTF8String);
  dev.rssi = RSSI ? RSSI.intValue : 0;

  AdvertisementData ad;
  NSString *localName = advertisementData[CBAdvertisementDataLocalNameKey];
  if (localName) ad.localName = std::string(localName.UTF8String);

  NSArray<CBUUID*> *svcUUIDs = advertisementData[CBAdvertisementDataServiceUUIDsKey];
  if (svcUUIDs) {
    for (CBUUID *u in svcUUIDs) {
      ad.serviceUuids.push_back(u.UUIDString.UTF8String);
      dev.serviceUuids.push_back(u.UUIDString.UTF8String);
    }
  }
  dev.ad = std::move(ad);

  if (_emit) _emit(Event{EScanResult{std::move(dev)}});
}
@end

namespace {

struct AppleAdapter : public IPlatformAdapter {
  std::function<void(const Event&)> sink_;
  BleCentralDelegate *delegate_ = nil;
  std::atomic<bool> scanning_{false};

  AppleAdapter() {
    delegate_ = [[BleCentralDelegate alloc] initWithEmitter:^(const Event &e){
      if (sink_) sink_(e);
    }];
    delegate_.mgr = [[CBCentralManager alloc] initWithDelegate:delegate_ queue:dispatch_get_main_queue()];
  }

  ~AppleAdapter() override {
    [delegate_.mgr stopScan];
    delegate_ = nil;
  }

  AdapterState adapterState() override {
    if (!delegate_.mgr) return AdapterState::Unknown;
    switch (delegate_.mgr.state) {
      case CBManagerStatePoweredOn:  return AdapterState::PoweredOn;
      case CBManagerStatePoweredOff: return AdapterState::PoweredOff;
      case CBManagerStateUnauthorized: return AdapterState::Unauthorized;
      case CBManagerStateUnsupported: return AdapterState::Unsupported;
      case CBManagerStateResetting: return AdapterState::Resetting;
      default: return AdapterState::Unknown;
    }
  }

  void setEventSink(std::function<void(const Event&)> sink) override { sink_ = std::move(sink); }

  std::error_code startScan(const ScanOptions& opts) override {
    if (scanning_.exchange(true)) return {};
    NSMutableArray<CBUUID*> *filter = nil;
    if (!opts.serviceUuids.empty()) {
      filter = [NSMutableArray new];
      for (auto &s : opts.serviceUuids) [filter addObject:[CBUUID UUIDWithString:[NSString stringWithUTF8String:s.c_str()]]];
    }
    NSDictionary *scanOpts = @{ CBCentralManagerScanOptionAllowDuplicatesKey : @(opts.allowDuplicates) };
    [delegate_.mgr scanForPeripheralsWithServices:filter options:scanOpts];
    return {};
  }

  void stopScan() override {
    if (!scanning_.exchange(false)) return;
    [delegate_.mgr stopScan];
  }

  // Stubs for now (implement later)
  std::error_code connect(const DeviceId&, const ConnectOptions&) override { return {}; }
  std::error_code disconnect(const DeviceId&) override { return {}; }
  std::error_code discover(const DeviceId&) override { return {}; }
  std::error_code read(const DeviceId&, const Uuid&, const Uuid&, ByteArray&) override { return {}; }
  std::error_code write(const DeviceId&, const Uuid&, const Uuid&, const ByteArray&, const WriteOptions&) override { return {}; }
  std::error_code setNotify(const DeviceId&, const Uuid&, const Uuid&, bool) override { return {}; }
  std::error_code readDescriptor(const DeviceId&, const Uuid&, const Uuid&, const Uuid&, ByteArray&) override { return {}; }
  std::error_code writeDescriptor(const DeviceId&, const Uuid&, const Uuid&, const Uuid&, const ByteArray&) override { return {}; }
  std::error_code readRssi(const DeviceId&, int&) override { return {}; }
  std::error_code requestMtu(const DeviceId&, int) override { return {}; }
  std::error_code setConnectionPriority(const DeviceId&, PriorityOptions) override { return {}; }
};

} // namespace

// Factory you’ll call from your test app:
extern "C" std::shared_ptr<IPlatformAdapter> makeAppleAdapter() {
  return std::make_shared<AppleAdapter>();
}
