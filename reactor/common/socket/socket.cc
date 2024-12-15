#include "socket.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>  // bzero
#include <unistd.h>
#include "reactor/common/InetAddress/InetAddress.h"
#include "reactor/common/logger/logger.h"
using namespace reactor;

Socket::~Socket() {
  if (::close(sockfd_) < 0) {
    LOG_ERROR << "sockets::close err";
  } else {
    LOG_INFO << "sockets::close succ";
  }
}

void Socket::bindAddress(const InetAddress& addr) {
  int ret = ::bind(sockfd_,
                   static_cast<const sockaddr*>(static_cast<const void*>(&addr.getSockAddrInet())),
                   sizeof addr);
  if (ret < 0) {
    LOG_FATAL << "bindAddress err";
  }
}

void Socket::listen() {
  int ret = ::listen(sockfd_, SOMAXCONN);
  if (ret < 0) {
    LOG_FATAL << "listen err";
  }
}

int Socket::accept(InetAddress* peeraddr) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof addr);
  socklen_t addrlen = sizeof addr;
  int connfd = ::accept4(sockfd_,
                         static_cast<sockaddr*>(static_cast<void*>(&addr)),
                         &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);

  if (connfd < 0) {
    int savedErrno = errno;
    LOG_ERROR << "accept4 err";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPROTO:  // ???
      case EPERM:
      case EMFILE:  // per-process lmit of open file desctiptor ???
        // expected errors
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        // unexpected errors
        LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default:
        LOG_FATAL << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  if (connfd >= 0) {
    peeraddr->setSockAddrInet(addr);
  }
  return connfd;
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::shutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_ERROR << "sockets::shutdownWrite error";
  }
}
