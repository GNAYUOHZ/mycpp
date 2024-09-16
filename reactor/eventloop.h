#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace reactor {
class Channel;
class Poller;
class TimerQueue;

class EventLoop {
 public:
  typedef std::function<void()> EventCallback;
  EventLoop();
  ~EventLoop();
  EventLoop(const EventLoop&) = delete;
  EventLoop& operator=(const EventLoop&) = delete;

  void loop();

  void quit();

  // timers

  /// Runs callback at 'time'.
  void runAt(int64_t timestamp, const EventCallback& cb);
  /// Runs callback after @c delay seconds.
  void runAfter(int64_t delay, const EventCallback& cb);
  /// Runs callback every @c interval seconds.
  void runEvery(int64_t interval, const EventCallback& cb);

  // internal use only
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

 private:
  bool looping_; /* atomic */
  bool quit_;    /* atomic */
  const std::thread::id threadId_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timerQueue_;
};

}  // namespace reactor
