#pragma once

#include <atomic>
#include <cstdint>
#include <condition_variable>

namespace stdlike {

class CondVar {
 public:
  // Mutex - BasicLockable
  template <class Lock>
  void Wait(Lock& lock) {
    if constexpr (has_atomic_wait_v<>) {
      uint64_t cur = seq_.load(std::memory_order_relaxed);
      lock.unlock();
      std::atomic_wait(seq_, cur);
      lock.lock();
    } else {
      cv_.wait(lock);
    }
  }

  void NotifyOne() {
    if constexpr (has_atomic_wait_v<>) {
      seq_.fetch_add(1, std::memory_order_release);
      std::atomic_notify_one(seq_);
    } else {
      cv_.notify_one();
    }
  }

  void NotifyAll() {
    if constexpr (has_atomic_wait_v<>) {
      seq_.fetch_add(1, std::memory_order_release);
      std::atomic_notify_all(seq_);
    } else {
      cv_.notify_all();
    }
  }

 private:
  template <typename A = std::atomic<uint64_t>>
  static constexpr bool has_atomic_wait_v = requires(A& a, uint64_t v) {
    std::atomic_wait(a, v);
    std::atomic_notify_one(a);
    std::atomic_notify_all(a);
  };

  std::atomic<uint64_t> seq_{0};
  std::condition_variable_any cv_;
};

}  // namespace stdlike