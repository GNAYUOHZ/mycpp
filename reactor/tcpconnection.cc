
#include "tcpconnection.h"

#include "channel.h"
#include "common/logger/logger.h"
#include "common/socket/socket.h"
#include "eventloop.h"

#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>

using namespace reactor;

static size_t writeBytesOnce_ = 4096;
TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(nameArg),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr) {
  LOG_INFO << "TcpConnection::ctor[" << name_ << "] at " << this << " fd=" << sockfd;
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
  LOG_INFO << "TcpConnection::dtor[" << name_ << "] at " << this << " fd=" << channel_->fd();
}

void TcpConnection::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(socket_->fd(), IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void TcpConnection::connectEstablished() {
  setState(kConnected);
  channel_->enableReading();
  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  setState(kDisconnected);
  // channel_->disableAll();
  connectionCallback_(shared_from_this());
  loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(int64_t receiveTime) {
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0) {
    messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  } else if (n == 0) {
    LOG_INFO << "TcpConnection::handleRead [" << name_ << "] - peer fd = " << channel_->fd()
             << " disconnected";
    handleClose();
  } else {
    errno = savedErrno;
    LOG_ERROR << "TcpConnection::handleRead error";
    handleError();
  }
}

void TcpConnection::handleWrite() {
  if (!channel_->isWriting()) {
    LOG_ERROR << "Connection is down, no more writing";
    return;
  }
  LOG_INFO << "TcpConnection::handleWrite";
  ssize_t n = ::write(channel_->fd(),
                      outputBuffer_.peek(),
                      std::min(outputBuffer_.readableBytes(), writeBytesOnce_));
  if (n > 0) {
    outputBuffer_.retrieve(n);
    if (outputBuffer_.readableBytes() == 0) {
      channel_->disableWriting();
      if (writeCompleteCallback_) {
        writeCompleteCallback_(shared_from_this());
      }
      if (state_ == kDisconnecting) {
        shutdown();
      }
    } else {
      LOG_DEBUG << "I am going to write more data, remain data len: "
                << outputBuffer_.readableBytes();
    }
  } else {
    LOG_ERROR << "TcpConnection::handleWrite error";
  }
}

void TcpConnection::handleClose() {
  LOG_INFO << "TcpConnection::handleClose state = " << state_;
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  channel_->disableAll();
  // must be the last line
  closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
  int err = 0;
  int optval;
  socklen_t optlen = sizeof optval;

  if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    err = errno;
  } else {
    err = optval;
  }
  LOG_ERROR << "TcpConnection::handleError [" << name_ << "] - SO_ERROR = " << err;
}

void TcpConnection::send(const std::string& message) {
  if (state_ != kConnected) {
    return;
  }
  ssize_t nwrote = 0;
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
    nwrote = ::write(channel_->fd(), message.data(), std::min(message.size(), writeBytesOnce_));
    if (nwrote >= 0) {
      if (static_cast<size_t>(nwrote) < message.size()) {
        LOG_DEBUG << "I am going to write more data, remain data len: " << message.size() - nwrote;
      } else if (writeCompleteCallback_) {
        writeCompleteCallback_(shared_from_this());
      }
    }
  } else {
    nwrote = 0;
    if (errno != EWOULDBLOCK) {
      LOG_ERROR << "TcpConnection::send error";
    }
  }

  if (static_cast<size_t>(nwrote) < message.size()) {
    outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
    if (!channel_->isWriting()) {
      channel_->enableWriting();
    }
  }
}

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
    return;
  }
  if (!channel_->isWriting()) {
    // we are not writing
    socket_->shutdownWrite();
  }
}
