#pragma once

#include <set>
#include <vector>
#include "channel.h"

namespace reactor {

class EventLoop;
class Timer;

class TimerQueue {
 public:
  using EventCallback = std::function<void()>;
  TimerQueue(EventLoop* loop);
  ~TimerQueue();
  TimerQueue(const TimerQueue&) = delete;
  TimerQueue& operator=(const TimerQueue&) = delete;

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  void addTimer(const EventCallback& cb, int64_t when, int64_t interval);

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
