// src/core/EventBus.cpp
#include "../../include/Events.h"
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <atomic>

namespace ble {

class EventBus {
public:
  EventBus();
  ~EventBus();

  ListenerId add(std::function<void(const Event&)> cb);
  void remove(ListenerId id);
  void emit(const Event& e);

private:
  std::mutex mu_;
  std::unordered_map<ListenerId, std::function<void(const Event&)>> listeners_;
  std::atomic<ListenerId> nextId_{1};
};

// ---- out-of-line definitions so the linker gets real symbols ----
EventBus::EventBus() = default;
EventBus::~EventBus() = default;

ListenerId EventBus::add(std::function<void(const Event&)> cb) {
  std::lock_guard<std::mutex> lock(mu_);
  ListenerId id = nextId_++;
  listeners_.emplace(id, std::move(cb));
  return id;
}

void EventBus::remove(ListenerId id) {
  std::lock_guard<std::mutex> lock(mu_);
  listeners_.erase(id);
}

void EventBus::emit(const Event& e) {
  // Copy callbacks so we call outside the lock
  std::vector<std::function<void(const Event&)>> cbs;
  {
    std::lock_guard<std::mutex> lock(mu_);
    cbs.reserve(listeners_.size());
    for (auto &kv : listeners_) cbs.push_back(kv.second);
  }
  for (auto &cb : cbs) cb(e);
}

} // namespace ble
