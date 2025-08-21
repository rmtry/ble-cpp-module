#pragma once
#include <future>
#include <system_error>

namespace ble {

// Simple Result<T> using std::future; value or throws std::system_error
template <typename T>
using Future = std::future<T>;

// Void alias
using FutureVoid = std::future<void>;

} // namespace ble
