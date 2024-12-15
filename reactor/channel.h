#pragma once

#include <functional>
#include <memory>

namespace reactor {

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(int64_t)>;

  Channel(EventLoop* loop, int fd);

  void handleEvent(int64_t receiveTime);
  void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb) { writeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb) { errorCallback_ = cb; }
  void setCloseCallback(const EventCallback& cb) { closeCallback_ = cb; }

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading();
  void enableWriting();
  void disableWriting();
  void disableAll();
  bool isWriting() const { return events_ & kWriteEvent; }

  // for Poller
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  EventLoop* ownerLoop() { return loop_; }

  std::string eventsToString();

 private:
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  int events_;   // 关⼼的IO事件
  int revents_;  // ⽬前活动的 事件
  int index_;    // used by Poller.

  bool eventHandling_;

  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback errorCallback_;
  EventCallback closeCallback_;
};

}  // namespace reactor
