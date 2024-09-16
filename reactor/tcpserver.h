#pragma once
#include <functional>
#include <map>
#include <memory>
#include "InetAddress.h"
#include "tcpconnection.h"

namespace reactor {

class Acceptor;
class EventLoop;

class TcpServer {
  typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
  typedef std::function<void(const TcpConnectionPtr&, Buffer* buf, int64_t)> MessageCallback;

 public:
  TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  ~TcpServer();
  TcpServer(const TcpServer&) = delete;
  TcpServer& operator=(const TcpServer&) = delete;
  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr);
  void removeConnection(const TcpConnectionPtr& conn);

  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

  EventLoop* loop_;  // the acceptor loop
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;  // avoid revealing Acceptor
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool started_;
  int nextConnId_;  // always in loop thread
  ConnectionMap connections_;
};

}  // namespace reactor
