#import <Foundation/Foundation.h>
#include "../include/BleClient.h"
#include "../include/Adapter.h"

extern std::shared_ptr<ble::IPlatformAdapter> makeAppleAdapter();

int main() {
  @autoreleasepool {
    auto adapter   = makeAppleAdapter();
    auto clientUp  = ble::BleClient::create(adapter);  // unique_ptr
    ble::BleClient* client = clientUp.get();           // non-owning raw pointer

    // Graceful SIGINT/SIGTERM via GCD signal sources (don’t use raw signal handlers for ObjC)
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    __block bool quitting = false;
    auto stopAndQuit = ^{
        if (quitting) return;
        quitting = true;

        printf("\nSIGINT — stopping scan & exiting...\n");
        client->stopScan();                       // your cleanup
        CFRunLoopStop(CFRunLoopGetMain());        // stop main runloop
        fflush(stdout);

        // ensure we actually terminate since we've disabled default SIGINT behavior
        dispatch_async(dispatch_get_main_queue(), ^{
            exit(0);
        });
    };

    dispatch_source_t sigint  = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGINT, 0, dispatch_get_main_queue());
    dispatch_source_t sigterm = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGTERM, 0, dispatch_get_main_queue());
    dispatch_source_set_event_handler(sigint,  stopAndQuit);
    dispatch_source_set_event_handler(sigterm, stopAndQuit);
    dispatch_resume(sigint);
    dispatch_resume(sigterm);

    bool started = false;

    client->onEvent([&](const ble::Event& e){
      if (auto st = std::get_if<ble::EAdapterStateChanged>(&e)) {
        printf("Adapter state %d\n", (int)st->state);
        if (st->state == ble::AdapterState::PoweredOn && !started) {
          ble::ScanOptions opts; opts.allowDuplicates = false;
          auto ec = client->startScan(opts);
          if (ec) {
            printf("startScan error: %d %s\n", (int)ec.value(), ec.message().c_str());
          } else {
            printf("Scanning started...\n");
            started = true;
          }
        }
      } else if (auto r = std::get_if<ble::EScanResult>(&e)) {
        const auto &d = r->device;
        printf("Found %s  name=%s  rssi=%d\n",
               d.id.c_str(),
               d.name ? d.name->c_str() : "(null)",
               d.rssi.value_or(0));
      }
    });

    [[NSRunLoop mainRunLoop] run];

    // unique_ptr cleans up after runloop stops
    return 0;
  }
}