#pragma once
#include <system_error>
namespace ble {

enum class Error {
  None = 0,
  NotReady,
  Unsupported,
  Unauthorized,
  InvalidArgument,
  Timeout,
  Canceled,
  Busy,
  DeviceNotFound,
  ConnectFailed,
  GattFailure,
  CharacteristicNotFound,
  DescriptorNotFound,
  AlreadyConnected,
  NotConnected,
  SubscriptionExists,
  NotSubscribed
};

const std::error_category& errorCategory();
inline std::error_code make_error(Error e) {
  return {static_cast<int>(e), errorCategory()};
}

} // namespace ble

// Provide std::is_error_code_enum specialization if desired
