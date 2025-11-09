#pragma once

#include "future.hpp"

#include <memory>
#include <utility>
#include <cassert>
#include <exception>

namespace stdlike {

    template <typename T>
    class Promise {
         public:
          Promise() : state_(std::make_shared<detail::SharedState<T>>()) {
          }

          // Non-copyable
          Promise(const Promise&) = delete;
          Promise& operator=(const Promise&) = delete;

          // Movable
          Promise(Promise&&) = default;
          Promise& operator=(Promise&&) = default;

          // One-shot
          Future<T> MakeFuture() {
            Future<T> f;
            f.state_ = state_;
            return f;
          }

          // One-shot
          // Fulfill promise with value
          void SetValue(T value) {
            assert(state_ && "Promise moved-from or invalid");
            auto s = state_;
            {
              std::lock_guard<std::mutex> lk(s->m);
              assert(!s->ready && "SetValue/SetException called more than once (UB)");
              s->result = std::move(value);
              s->ready = true;
            }
            s->cv.notify_all();
          }

          // One-shot
          // Fulfill promise with exception
          void SetException(std::exception_ptr ex) {
            assert(state_ && "Promise moved-from or invalid");
            auto s = state_;
            {
              std::lock_guard<std::mutex> lk(s->m);
              assert(!s->ready && "SetValue/SetException called more than once (UB)");
              s->result = ex;
              s->ready = true;
            }
            s->cv.notify_all();
          }

         private:
          std::shared_ptr<detail::SharedState<T>> state_;
    };

}  // namespace stdlike
