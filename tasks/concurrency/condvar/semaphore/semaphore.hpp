#pragma once

#include <mutex>
#include <condition_variable>
#include <cstdint>

namespace solutions {

// A counting semaphore (basic, condition-variable based)
class Semaphore {
 public:
  // Creates a Semaphore with the given number of permits
  explicit Semaphore(size_t initial) : permits_(static_cast<uint32_t>(initial)) {}

  // Acquires a permit from this semaphore, blocking until one is available
  void Acquire() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() { return permits_ != 0; });
    --permits_;
  }

  // Releases a permit, returning it to the semaphore
  void Release() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      ++permits_;
    }
    cond_.notify_one();
  }

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  uint32_t permits_{0};
};

}  // namespace solutions
