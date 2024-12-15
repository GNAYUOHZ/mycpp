#pragma once

#include <set>
#include <vector>
#include "channel.h"

namespace reactor {

class EventLoop;

class Timer {
  using TimerEventCallback = std::function<void()>;

 public:
  Timer(const TimerEventCallback& cb, int64_t when, double interval)
      : callback_(cb), expiration_(when), interval_(interval), repeat_(interval > 0.0) {}

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

  void run() const { callback_(); }
  int64_t expiration() const { return expiration_; }
  bool repeat() const { return repeat_; }
  void restart(int64_t now) { expiration_ = now + interval_; }

 private:
  const TimerEventCallback callback_;
  int64_t expiration_;
  const double interval_;
  const bool repeat_;
};

class TimerQueue {
  using TimerEventCallback = std::function<void()>;

 public:
  TimerQueue(EventLoop* loop);
  ~TimerQueue();
  TimerQueue(const TimerQueue&) = delete;
  TimerQueue& operator=(const TimerQueue&) = delete;

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  void addTimer(const TimerEventCallback& cb, int64_t when, int64_t interval);

  // void cancel(TimerId timerId);

 private:
  typedef std::pair<int64_t, Timer*> Entry;
  typedef std::set<Entry> TimerList;

  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  std::vector<Entry> getExpired(int64_t now);

  bool insert(Timer* timer);

  int timerfd_;
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;
};

}  // namespace reactor
