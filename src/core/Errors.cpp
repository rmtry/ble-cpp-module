// src/core/Errors.cpp
#include "../../include/Errors.h"
#include <string>

namespace ble {

namespace {
struct BleErrorCategory final : std::error_category {
  const char* name() const noexcept override { return "ble"; }
  std::string message(int ev) const override {
    switch (static_cast<Error>(ev)) {
      case Error::None: return "no error";
      case Error::NotReady: return "not ready";
      case Error::Unsupported: return "unsupported";
      case Error::Unauthorized: return "unauthorized";
      case Error::InvalidArgument: return "invalid argument";
      case Error::Timeout: return "timeout";
      case Error::Canceled: return "canceled";
      case Error::Busy: return "busy";
      case Error::DeviceNotFound: return "device not found";
      case Error::ConnectFailed: return "connect failed";
      case Error::GattFailure: return "gatt failure";
      case Error::CharacteristicNotFound: return "characteristic not found";
      case Error::DescriptorNotFound: return "descriptor not found";
      case Error::AlreadyConnected: return "already connected";
      case Error::NotConnected: return "not connected";
      case Error::SubscriptionExists: return "subscription exists";
      case Error::NotSubscribed: return "not subscribed";
    }
    return "unknown";
  }
};
const BleErrorCategory& cat() {
  static BleErrorCategory c;
  return c;
}
} // namespace

const std::error_category& errorCategory() { return cat(); }

} // namespace ble
