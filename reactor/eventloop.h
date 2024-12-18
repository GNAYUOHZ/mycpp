#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

struct epoll_event;

namespace reactor {
class Channel;
class TimerQueue;

class EventLoop {
  typedef std::vector<Channel*> ChannelList;
  typedef std::vector<struct epoll_event> EventList;
  typedef std::function<void()> TimerEventCallback;

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
  void update(int operation, Channel* channel);

 private:
  bool looping_; /* atomic */
  std::unique_ptr<TimerQueue> timerQueue_;

  // epoll
  static const int kInitEventListSize = 16;
  EventList events_;
  int epollfd_;
};

}  // namespace reactor
