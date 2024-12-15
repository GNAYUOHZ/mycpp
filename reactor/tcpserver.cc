#include "tcpserver.h"

#include <stdio.h>  // snprintf
#include <strings.h>
#include <unistd.h>
#include "common/logger/logger.h"
#include "eventloop.h"

using namespace reactor;

TcpServer::TcpServer(const InetAddress& listenAddr)
    : loop_(),
      name_(listenAddr.toHostPort()),
      started_(false),
      nextConnId_(1),
      acceptSocket_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
      acceptChannel_(&loop_, acceptSocket_.fd()) {
  if (acceptSocket_.fd() < 0) {
    LOG_FATAL << "socket err";
  }

  acceptSocket_.setReuseAddr(true);
  acceptSocket_.bindAddress(listenAddr);

  acceptChannel_.setReadCallback(std::bind(&TcpServer::acceptConnection, this));
  LOG_DEBUG << "listen on " << listenAddr.toHostPort();
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
  if (!started_) {
    started_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
    loop_.loop();
  }
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  conn->connectDestroyed();
}

void TcpServer::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection() succ: new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());

  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void TcpServer::onMessage(const reactor::TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime) {
  size_t size = buf->readableBytes();
  std::string message = buf->retrieveAllAsString();
  printf("onMessage(): received %d bytes from connection [%s] at %ld, content: %s\n",
         size,
         conn->name().c_str(),
         receiveTime,
         message.c_str());
  conn->send(message);
}

void TcpServer::onWriteComplete(const reactor::TcpConnectionPtr& conn) {
  printf("onWriteComplete()\n");
}

void TcpServer::acceptConnection() {
  InetAddress peerAddr(0);
  int sockfd = acceptSocket_.accept(&peerAddr);
  if (sockfd < 0) {
    LOG_ERROR << "accept conn err";
  }

  char buf[32];
  snprintf(buf, sizeof buf, "#%d", nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;
  LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection [" << connName
           << "] from " << peerAddr.toHostPort();

  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof localaddr);
  socklen_t addrlen = sizeof(localaddr);

  if (::getsockname(sockfd, static_cast<sockaddr*>(static_cast<void*>(&localaddr)), &addrlen) < 0) {
    LOG_ERROR << "getsockname err";
  }

  TcpConnectionPtr conn(new TcpConnection(&loop_, connName, sockfd, localaddr, peerAddr));
  connections_[connName] = conn;

  conn->setConnectionCallback(std::bind(&TcpServer::onConnection, this, std::placeholders::_1));
  conn->setMessageCallback(std::bind(&TcpServer::onMessage,
                                     this,
                                     std::placeholders::_1,
                                     std::placeholders::_2,
                                     std::placeholders::_3));
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  conn->setWriteCompleteCallback(
      std::bind(&TcpServer::onWriteComplete, this, std::placeholders::_1));
  conn->connectEstablished();
}

void TcpServer::registTimer() {
  // test timerfd
  auto timeout = [](const char* msg) { LOG_INFO << "timeout " << msg; };
  int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
  loop_.runAfter(1000, std::bind(timeout, "once"));
  loop_.runEvery(2000, std::bind(timeout, "every"));
  loop_.runAt(now + 3000, std::bind(timeout, "at"));
}