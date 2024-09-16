#pragma once

#include <poll.h>
#include <unordered_map>
#include <vector>

namespace reactor {

class Channel;
class EventLoop;

///
/// IO Multiplexing with poll(2).
///
/// This class doesn't own the Channel objects.
class Poller {
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  ~Poller();

  /// Polls the I/O events.
  int64_t poll(int timeoutMs, ChannelList* activeChannels);

  /// Changes the interested I/O events.
  void updateChannel(Channel* channel);
  /// Remove the channel, when it destructs.
  void removeChannel(Channel* channel);

 private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  EventLoop* ownerLoop_;
  std::vector<struct pollfd> pollfds_;
  std::unordered_map<int, Channel*> channels_;
};

}  // namespace reactor
