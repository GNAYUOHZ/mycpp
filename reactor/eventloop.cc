#include "eventloop.h"

#include <signal.h>
#include <sys/syscall.h>
#include <chrono>
#include "channel.h"
#include "logger.h"
#include "poller.h"
#include "timerqueue.h"

using namespace reactor;

thread_local EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};

EventLoop::EventLoop()
    : looping_(false),
      threadId_(std::this_thread::get_id()),
      poller_(new Poller(this)),
      timerQueue_(new TimerQueue(this)) {
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
  quit_ = false;

  while (!quit_) {
    std::vector<Channel*> activeChannels_;
    int64_t pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    for (std::vector<Channel*>::iterator it = activeChannels_.begin(); it != activeChannels_.end();
         ++it) {
      (*it)->handleEvent(pollReturnTime_);
    }
  }

  LOG_INFO << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  // wakeup();
}

void EventLoop::updateChannel(Channel* channel) { poller_->updateChannel(channel); }

void EventLoop::removeChannel(Channel* channel) { poller_->removeChannel(channel); }

void EventLoop::runAt(int64_t timestamp, const EventCallback& cb) {
  timerQueue_->addTimer(cb, timestamp, 0);
}

void EventLoop::runAfter(int64_t delay, const EventCallback& cb) {
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  runAt(now + delay, cb);
}

void EventLoop::runEvery(int64_t interval, const EventCallback& cb) {
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  timerQueue_->addTimer(cb, now + interval, interval);
}