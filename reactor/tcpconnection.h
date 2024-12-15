#pragma once

#include <functional>
#include <memory>
#include "common/InetAddress/InetAddress.h"
#include "common/buffer/buffer.h"

namespace reactor {
class Channel;
class EventLoop;
class Socket;

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer* buf, int64_t)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
///
/// TCP connection, for both client and server usage.
///
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  /// User should not create this object.
  TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
  ~TcpConnection();
  TcpConnection(const TcpConnection&) = delete;
  TcpConnection& operator=(const TcpConnection&) = delete;

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() { return localAddr_; }
  const InetAddress& peerAddress() { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }

  void send(const std::string& message);
  void shutdown();
  void setTcpNoDelay(bool on);

  void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }

  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
  /// Internal use only.

  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }
  // called when TcpServer accepts a new connection
  void connectEstablished();  // should be called only once

  // called when TcpServer has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum StateE {
    kConnecting,
    kConnected,
    kDisconnecting,
    kDisconnected,
  };

  void setState(StateE s) { state_ = s; }
  void handleRead(int64_t receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  EventLoop* loop_;
  std::string name_;
  StateE state_;  // FIXME: use atomic variable
  // we don't expose those classes to client.
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  CloseCallback closeCallback_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;
};

}  // namespace reactor
