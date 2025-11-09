#pragma once

#include "tagged_semaphore.hpp"

#include <deque>
#include <utility>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue
template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t capacity)
      : spaces_(capacity), items_(0), mutex_(1) {}

  // Inserts the specified element into this queue,
  // waiting if necessary for space to become available.
  void Put(T value) {
    auto space_token = spaces_.Acquire();

    {
      auto guard = mutex_.MakeGuard();
      buffer_.push_back(std::move(value));
    }

    items_.Release(std::move(space_token));
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    auto item_token = items_.Acquire();

    T result;
    {
      auto guard = mutex_.MakeGuard();
      result = std::move(buffer_.front());
      buffer_.pop_front();
    }

    spaces_.Release(std::move(item_token));
    return result;
  }

 private:
  using SlotTag = struct QueueTag;
  using LockTag = struct MutexTag;

  TaggedSemaphore<SlotTag> spaces_;
  TaggedSemaphore<SlotTag> items_;
  TaggedSemaphore<LockTag> mutex_;
  std::deque<T> buffer_;
};

}  // namespace solutions
