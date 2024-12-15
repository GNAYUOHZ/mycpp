#pragma once
#include <functional>
#include <map>
#include <memory>
#include "channel.h"
#include "common/InetAddress/InetAddress.h"
#include "common/buffer/buffer.h"
#include "common/socket/socket.h"
#include "eventloop.h"
#include "tcpconnection.h"

namespace reactor {

class EventLoop;

class TcpServer {
 public:
  TcpServer(const InetAddress& listenAddr);
  ~TcpServer();
  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;
  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void removeConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime);
  void onWriteComplete(const TcpConnectionPtr& conn);

  void acceptConnection();

  void registTimer();

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop loop_;
  const std::string name_;
  bool started_;
  int nextConnId_;  // always in loop thread
  ConnectionMap connections_;
  Socket acceptSocket_;
  Channel acceptChannel_;
};

}  // namespace reactor
