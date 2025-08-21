#include "../include/BleClient.h"
#include "../include/Adapter.h"
#include <iostream>

int main() {
  using namespace ble;

  AdapterPtr adapter = /* makeAppleAdapter() / makeWinRtAdapter() / makeDummyAdapterForNow() */;
  auto client = BleClient::create(adapter);

  client->onEvent([](const Event& e){
    if (auto s = std::get_if<EScanResult>(&e)) {
      std::cout << "Device: " << s->device.id << " rssi=" << (s->device.rssi.value_or(0)) << "\n";
    }
    if (auto n = std::get_if<ENotification>(&e)) {
      std::cout << "Notify from " << n->characteristic << " size=" << n->value.size() << "\n";
    }
  });

  ScanOptions opts; opts.allowDuplicates = false;
  client->startScan(opts);

  // ... pick a deviceId after some results ...
  DeviceId id = "XX:XX:XX:XX:XX:XX";
  client->stopScan();

  client->connect(id, ConnectOptions{.autoConnect=false, .timeoutMs=10000}).get();
  auto services = client->discoverServices(id).get();

  // Read a characteristic
  Uuid svc = "0000180f-0000-1000-8000-00805f9b34fb"; // Battery Service
  Uuid chr = "00002a19-0000-1000-8000-00805f9b34fb"; // Battery Level
  auto data = client->read(id, svc, chr).get();
  std::cout << "Battery level: " << (int)data[0] << "%\n";

  client->disconnect(id).get();
}
