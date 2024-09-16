
#include "acceptor.h"

#include <unistd.h>
#include "InetAddress.h"
#include "eventloop.h"
#include "logger.h"

using namespace reactor;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      acceptSocket_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false) {
  if (acceptSocket_.fd() < 0) {
    LOG_FATAL << "socket err";
  }
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
  LOG_DEBUG << "listen on " << listenAddr.toHostPort();
}

void Acceptor::listen() {
  listenning_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  InetAddress peerAddr(0);
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    } else {
      if (::close(connfd) < 0) {
        LOG_ERROR << "close err";
      }
    }
  }
}
