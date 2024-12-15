#include "eventloop.h"

#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <chrono>
#include "channel.h"
#include "common/logger/logger.h"
#include "timerqueue.h"

using namespace reactor;

const int kPollTimeMs = 10000;

EventLoop::EventLoop()
    : looping_(false),
      timerQueue_(new TimerQueue(this)),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  LOG_INFO << "EventLoop created " << this << " in thread " << std::this_thread::get_id();
}
EventLoop::~EventLoop() { ::close(epollfd_); }

void EventLoop::loop() {
  looping_ = true;

  while (looping_) {
    int numEvents =
        ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), kPollTimeMs);
    int savedErrno = errno;
    int64_t pollReturnTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
    ChannelList activeChannels_;
    if (numEvents > 0) {
      LOG_DEBUG << numEvents << " events happended";
      // fillActiveChannels
      for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels_.push_back(channel);
      }
      if (static_cast<size_t>(numEvents) == events_.size()) {
        events_.resize(events_.size() * 2);
      }
    } else if (numEvents == 0) {
      LOG_DEBUG << "nothing happended";
    } else {
      if (savedErrno != EINTR) {
        LOG_ERROR << "epoll err";
      }
    }

    for (std::vector<Channel*>::iterator it = activeChannels_.begin(); it != activeChannels_.end();
         ++it) {
      (*it)->handleEvent(pollReturnTime_);
    }
  }

  LOG_INFO << "EventLoop " << this << " stop looping";
}

void EventLoop::quit() {
  looping_ = false;
  // wakeup();
}

void EventLoop::updateChannel(Channel* channel) {
  const int index = channel->index();
  LOG_ERROR << "fd = " << channel->fd() << " events = " << channel->events()
            << " index = " << index;
  if (index == kNew) {
    // a new one, add with EPOLL_CTL_ADD
    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  } else if (channel->isNoneEvent()) {
    update(EPOLL_CTL_DEL, channel);
    channel->set_index(kDeleted);
  } else {
    update(EPOLL_CTL_MOD, channel);
  }
}

void EventLoop::removeChannel(Channel* channel) {
  if (channel->index() == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kDeleted);
}

void EventLoop::update(int operation, Channel* channel) {
  struct epoll_event event;
  memset(&event, 0, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  static std::vector<std::string> opname = {"", "ADD", "DEL", "MOD"};
  LOG_INFO << "epoll_ctl op = " << opname[operation] << " fd = " << fd << " event = { "
           << channel->eventsToString() << " }";
  ::epoll_ctl(epollfd_, operation, fd, &event);
}

void EventLoop::runAt(int64_t timestamp, const TimerEventCallback& cb) {
  timerQueue_->addTimer(cb, timestamp, 0);
}

void EventLoop::runAfter(int64_t delay, const TimerEventCallback& cb) {
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  timerQueue_->addTimer(cb, now + delay, 0);
}

void EventLoop::runEvery(int64_t interval, const TimerEventCallback& cb) {
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  timerQueue_->addTimer(cb, now + interval, interval);
}
