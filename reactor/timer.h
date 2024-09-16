#pragma once
#include <functional>

namespace reactor {

///
/// Internal class for timer event.
///
class Timer {
  using EventCallback = std::function<void()>;

 public:
  Timer(const EventCallback& cb, int64_t when, double interval)
      : callback_(cb), expiration_(when), interval_(interval), repeat_(interval > 0.0) {}

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;
  void run() const { callback_(); }

  int64_t expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }

  void restart(int64_t now) { expiration_ = now + interval_; }

 private:
  const EventCallback callback_;
  int64_t expiration_;
  const double interval_;
  const bool repeat_;
};

}  // namespace reactor
