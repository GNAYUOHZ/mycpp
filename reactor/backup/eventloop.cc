#include "eventloop.h"

#include <signal.h>
#include <sys/syscall.h>
#include <chrono>
#include "channel.h"
#include "common/logger/logger.h"
#include "timerqueue.h"

using namespace reactor;

thread_local EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

EventLoop::EventLoop()
    : looping_(false), threadId_(std::this_thread::get_id()), timerQueue_(new TimerQueue(this)) {
  LOG_INFO << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread) {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread "
              << threadId_;
  } else {
    t_loopInThisThread = this;
  }
}

EventLoop::~EventLoop() { t_loopInThisThread = NULL; }

void EventLoop::loop() {
  looping_ = true;

  while (looping_) {
    std::vector<Channel*> activeChannels_;

    // XXX pollfds_ shouldn't change
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), kPollTimeMs);
    int64_t pollReturnTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::system_clock::now().time_since_epoch())
                                  .count();
    if (numEvents > 0) {
      LOG_DEBUG << numEvents << " events happended";
      fillActiveChannels(numEvents, &activeChannels_);
    } else if (numEvents == 0) {
      LOG_DEBUG << "nothing happended";
    } else {
      LOG_ERROR << "Poller::poll() err";
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

void EventLoop::runAt(int64_t timestamp, const TimerEventCallback& cb) {
  timerQueue_->addTimer(cb, timestamp, 0);
}

void EventLoop::runAfter(int64_t delay, const TimerEventCallback& cb) {
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  runAt(now + delay, cb);
}

void EventLoop::runEvery(int64_t interval, const TimerEventCallback& cb) {
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  timerQueue_->addTimer(cb, now + interval, interval);
}

void EventLoop::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
  for (std::vector<struct pollfd>::const_iterator pfd = pollfds_.begin();
       pfd != pollfds_.end() && numEvents > 0;
       ++pfd) {
    if (pfd->revents > 0) {
      --numEvents;
      std::unordered_map<int, Channel*>::const_iterator ch = channels_.find(pfd->fd);
      Channel* channel = ch->second;
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      activeChannels->push_back(channel);
    }
  }
}

void EventLoop::updateChannel(Channel* channel) {
  if (channel->index() < 0) {
    // a new one, add to pollfds_
    struct pollfd pfd;

    pfd.fd = channel->fd();
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;

    pollfds_.push_back(pfd);
    channel->set_index(static_cast<int>(pollfds_.size()) - 1);
    channels_[pfd.fd] = channel;
  } else {
    // update existing one
    int idx = channel->index();
    struct pollfd& pfd = pollfds_[idx];
    pfd.events = static_cast<short>(channel->events());
    pfd.revents = 0;
    if (channel->isNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -1;
    }
  }
}

void EventLoop::removeChannel(Channel* channel) {
  int idx = channel->index();
  const struct pollfd& pfd = pollfds_[idx];

  size_t n = channels_.erase(channel->fd());
  if (static_cast<size_t>(idx) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  } else {
    int channelAtEnd = pollfds_.back().fd;
    iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd - 1;
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}
