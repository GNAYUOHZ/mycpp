#include "tcpserver.h"

#include "acceptor.h"
#include "eventloop.h"
#include "logger.h"

#include <stdio.h>  // snprintf
#include <strings.h>

using namespace reactor;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      name_(listenAddr.toHostPort()),
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1) {
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
  started_ = true;

  if (!acceptor_->listenning()) {
    acceptor_->listen();
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
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

  // FIXME poll with zero timeout to double confirm the new connection
  TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localaddr, peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << "TcpServer::removeConnection [" << name_ << "] - connection " << conn->name();
  size_t n = connections_.erase(conn->name());
  conn->connectDestroyed();
}
