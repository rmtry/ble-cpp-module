#import <Foundation/Foundation.h>
#include "../include/BleClient.h"
#include "../include/Adapter.h"

extern std::shared_ptr<ble::IPlatformAdapter> makeAppleAdapter();

int main() {
  @autoreleasepool {
    auto adapter = makeAppleAdapter();
    auto client  = ble::BleClient::create(adapter);

    client->onEvent([](const ble::Event& e){
      if (auto st = std::get_if<ble::EAdapterStateChanged>(&e)) {
        printf("Adapter state: %d\n", (int)st->state);
      } else if (auto r = std::get_if<ble::EScanResult>(&e)) {
        const auto &d = r->device;
        printf("Found %s  name=%s  rssi=%d\n",
               d.id.c_str(),
               d.name ? d.name->c_str() : "(null)",
               d.rssi.value_or(0));
      }
    });

    ble::ScanOptions opts;
    opts.allowDuplicates = false;
    auto ec = client->startScan(opts);
    if (ec) { fprintf(stderr, "startScan error: %d\n", (int)ec.value()); return 1; }

    // Keep the main thread alive to receive delegate callbacks.
    [[NSRunLoop mainRunLoop] run];
  }
  return 0;
}
