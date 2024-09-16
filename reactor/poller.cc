#include "poller.h"

#include "channel.h"
#include "eventloop.h"
#include "logger.h"

using namespace reactor;

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) {}

Poller::~Poller() {}

int64_t Poller::poll(int timeoutMs, ChannelList* activeChannels) {
  // XXX pollfds_ shouldn't change
  int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  if (numEvents > 0) {
    LOG_DEBUG << numEvents << " events happended";
    fillActiveChannels(numEvents, activeChannels);
  } else if (numEvents == 0) {
    LOG_DEBUG << " nothing happended";
  } else {
    LOG_ERROR << "Poller::poll() err";
  }
  return now;
}

void Poller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
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

void Poller::updateChannel(Channel* channel) {
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

void Poller::removeChannel(Channel* channel) {
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
