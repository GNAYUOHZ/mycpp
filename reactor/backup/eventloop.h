#pragma once
#include <poll.h>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace reactor {
class Channel;
class TimerQueue;

class EventLoop {
  typedef std::vector<Channel*> ChannelList;

 public:
  EventLoop();
  ~EventLoop();
  EventLoop(const EventLoop&) = delete;
  EventLoop& operator=(const EventLoop&) = delete;

  void loop();

  void quit();

  // timers

  /// Runs callback at 'time'.
  void runAt(int64_t timestamp, const TimerEventCallback& cb);
  /// Runs callback after @c delay seconds.
  void runAfter(int64_t delay, const TimerEventCallback& cb);
  /// Runs callback every @c interval seconds.
  void runEvery(int64_t interval, const TimerEventCallback& cb);

  // internal use only
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);

 private:
  /// Changes the interested I/O events.
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

 private:
  bool looping_; /* atomic */
  const std::thread::id threadId_;
  std::unique_ptr<TimerQueue> timerQueue_;
  std::vector<struct pollfd> pollfds_;
  std::unordered_map<int, Channel*> channels_;
};

}  // namespace reactor
