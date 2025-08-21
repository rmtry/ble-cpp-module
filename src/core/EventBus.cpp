#include "../include/Events.h"
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <atomic>

namespace ble {

// Forward-declare the EventBus API so BleClient.cpp can link against it.
class EventBus {
public:
  EventBus() = default;
  ~EventBus() = default;

  ListenerId add(std::function<void(const Event&)> cb) {
    std::lock_guard<std::mutex> lock(mu_);
    ListenerId id = nextId_++;
    listeners_.emplace(id, std::move(cb));
    return id;
  }

  void remove(ListenerId id) {
    std::lock_guard<std::mutex> lock(mu_);
    listeners_.erase(id);
  }

  void emit(const Event& e) {
    // Copy callbacks so we call outside the lock.
    std::vector<std::function<void(const Event&)>> cbs;
    {
      std::lock_guard<std::mutex> lock(mu_);
      cbs.reserve(listeners_.size());
      for (auto &kv : listeners_) cbs.push_back(kv.second);
    }
    for (auto &cb : cbs) cb(e);
  }

private:
  std::mutex mu_;
  std::unordered_map<ListenerId, std::function<void(const Event&)>> listeners_;
  std::atomic<ListenerId> nextId_{1};
};

} // namespace ble
