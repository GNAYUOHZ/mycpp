#include "channel.h"
#include <sstream>
#include "eventloop.h"
#include "logger.h"

#include <poll.h>

using namespace reactor;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1) {
  LOG_INFO << "new channel, fd = " << fd_;
}


 // todo 需处理close问题
void Channel::handleEvent(int64_t receiveTime) {
  eventHandling_ = true;
  if (revents_ & POLLNVAL) {
    LOG_DEBUG << "Channel::handle_event() POLLNVAL";
  }

  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    LOG_DEBUG << "Channel::handle_event() closeCallback_";
    if (closeCallback_) closeCallback_();
    return;
  }
  if (revents_ & (POLLERR | POLLNVAL)) {
    LOG_ERROR << "Channel::handle_event() errorCallback_";
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
    LOG_ERROR << "Channel::handle_event() readCallback_";
    if (readCallback_) readCallback_(receiveTime);
  }
  if (revents_ & POLLOUT) {
    LOG_ERROR << "Channel::handle_event() writeCallback_";
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

void Channel::enableReading() {
  events_ |= kReadEvent;
  loop_->updateChannel(this);
  LOG_INFO << "enableReading, fd = " << fd() << " events = " << events();
}

void Channel::disableAll() {
  events_ = kNoneEvent;
  loop_->updateChannel(this);
  LOG_INFO << "disableAll, fd = " << fd() << " events = " << events();
}

void Channel::enableWriting() {
  events_ |= kWriteEvent;
  loop_->updateChannel(this);
  LOG_INFO << "enableWriting, fd = " << fd() << " events = " << events();
}
void Channel::disableWriting() {
  events_ &= ~kWriteEvent;
  loop_->updateChannel(this);
  LOG_INFO << "disableWriting, fd = " << fd() << " events = " << events();
}
