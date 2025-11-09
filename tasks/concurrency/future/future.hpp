#pragma once

#include <memory>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include <variant>
#include <exception>
#include <type_traits>
#include <utility>

namespace stdlike {

    namespace detail {

        template <typename T>
        struct SharedState {
          using ResultVariant =
              std::conditional_t<std::is_same_v<T, std::monostate>,
                                 std::variant<std::monostate, std::exception_ptr>,
                                 std::variant<std::monostate, T, std::exception_ptr>>;

          std::mutex m;
          std::condition_variable cv;
          std::variant<std::monostate, T, std::exception_ptr> result;
          bool ready = false;

          bool Ready() const { return ready; }
        };

    }  // namespace detail

    template <typename T>
        class Future {
          template <typename U>
          friend class Promise;

         public:
          // Non-copyable
          Future(const Future&) = delete;
          Future& operator=(const Future&) = delete;

          // Movable
          Future(Future&&) = default;
          Future& operator=(Future&&) = default;

          // One-shot
          // Wait for result (value or exception)
          T Get() {
            assert(state_ && "Get() called on invalid Future");
            auto s = state_;
            std::unique_lock<std::mutex> lk(s->m);
            s->cv.wait(lk, [&] { return s->Ready(); });

            if constexpr (std::is_same_v<T, std::monostate>) {
              if (std::holds_alternative<std::exception_ptr>(s->result)) {
                std::exception_ptr ex = std::get<std::exception_ptr>(std::move(s->result));
                std::rethrow_exception(ex);
              }
              return std::monostate{};
            } else {
              if (std::holds_alternative<std::exception_ptr>(s->result)) {
                std::exception_ptr ex = std::get<std::exception_ptr>(std::move(s->result));
                std::rethrow_exception(ex);
              }
              return std::get<T>(std::move(s->result));
            }
          }

         private:
          Future() = default;

         private:
          std::shared_ptr<detail::SharedState<T>> state_;
    };

}  // namespace stdlike
