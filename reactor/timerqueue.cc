
#include "timerqueue.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include "eventloop.h"
#include "logger.h"
#include "timer.h"

using namespace reactor;

void readTimerfd(int timerfd) {
  uint64_t howmany;
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_DEBUG << "TimerQueue::handleRead() " << howmany << " at " << now;
  if (n != sizeof howmany) {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, int64_t expiration) {
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, 0, sizeof(newValue));
  memset(&oldValue, 0, sizeof(oldValue));
  // Get current system time in milliseconds
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();

  // Calculate the difference in milliseconds
  int64_t diff = expiration - now;

  // Set the timer expiration
  newValue.it_value.tv_sec = diff / 1000;               // seconds
  newValue.it_value.tv_nsec = (diff % 1000) * 1000000;  // nanoseconds

  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret) {
    LOG_ERROR << "timerfd_settime() error: " << ret;
  }
}

TimerQueue::TimerQueue(EventLoop* loop)
    : timerfd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
      timerfdChannel_(loop, timerfd_),
      timers_() {
  timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
  timerfdChannel_.enableReading();
  LOG_DEBUG << "starting TimerQueue";
}

TimerQueue::~TimerQueue() {
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it) {
    delete it->second;
  }
}

void TimerQueue::addTimer(const EventCallback& cb, int64_t when, int64_t interval) {
  Timer* timer = new Timer(cb, when, interval);
  bool earliestChanged = insert(timer);

  if (earliestChanged) {
    resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::handleRead() {
  readTimerfd(timerfd_);
  // Get current system time in milliseconds
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  std::vector<Entry> expired = getExpired(now);

  // safe to callback outside critical section
  for (std::vector<Entry>::iterator it = expired.begin(); it != expired.end(); ++it) {
    it->second->run();
  }

  // reset
  for (std::vector<Entry>::const_iterator it = expired.begin(); it != expired.end(); ++it) {
    if (it->second->repeat()) {
      it->second->restart(now);
      insert(it->second);
    } else {
      delete it->second;
    }
  }
  if (!timers_.empty()) {
    int64_t nextExpire = timers_.begin()->second->expiration();
    resetTimerfd(timerfd_, nextExpire);
  }
}
std::vector<TimerQueue::Entry> TimerQueue::getExpired(int64_t now) {
  std::vector<Entry> expired;
  Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator it = timers_.lower_bound(sentry);
  std::copy(timers_.begin(), it, back_inserter(expired));
  timers_.erase(timers_.begin(), it);
  return expired;
}

bool TimerQueue::insert(Timer* timer) {
  bool earliestChanged = false;
  int64_t when = timer->expiration();
  TimerList::iterator it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliestChanged = true;
  }
  std::pair<TimerList::iterator, bool> result = timers_.insert(std::make_pair(when, timer));
  return earliestChanged;
}
