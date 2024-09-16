#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <string>

#include "anet.h"
#include "epoll/epoll.h"

/* Error codes */
#define C_OK 0
#define C_ERR -1

/* Synchronous I/O with timeout */
ssize_t syncWrite(int fd, char *ptr, ssize_t size, long long timeout);
ssize_t syncRead(int fd, char *ptr, ssize_t size, long long timeout);
ssize_t syncReadLine(int fd, char *ptr, ssize_t size, long long timeout);

class TcpServer {
 public:
  TcpServer(int port);

  int Loop() { eventLoop.aeMain(); }

 private:
  // void acceptTcpHandler(int fd, void *privdata, int mask) {
  //   std::string cip;
  //   cip.resize(NET_IP_STR_LEN);
  //   int cport;
  //   int maxAccepts = MAX_ACCEPTS_PER_CALL;
  //   while (maxAccepts--) {
  //     int cfd = anetTcpAccept(fd, &cip[0], cip.size(), &cport);
  //     if (cfd == ANET_ERR) {
  //       if (errno != EWOULDBLOCK) {
  //         std::cerr << "Accepting client connection err" << std::endl;
  //       }
  //       return;
  //     }
  //     // Truncate the string to the actual length of the IP string.
  //     cip.resize(std::strlen(cip.c_str()));
  //     std::cout << "Accepted " << cip << ":" << cport << std::endl;

  //     // connCreateAcceptedSocket
  //     connection *conn = new connection;
  //     //   conn->type = &CT_Socket;  // TODO
  //     conn->fd = fd;
  //     conn->state = CONN_STATE_ACCEPTING;

  //     // acceptCommonHandler
  //     client *c;
  //     char conninfo[100];
  //     /* Limit the number of connections we take at the same time.
  //      *
  //      * Admission control will happen before a client is created and
  //      * connAccept() called, because we don't want to even start
  //      * transport-level negotiation if rejected. */

  //     /* Create connection and client */
  //     client *c = new client;
  //   }
  // }

  // /* Accept a connection and also make sure the socket is non-blocking, and
  //  * CLOEXEC. returns the new socket FD, or -1 on error. */
  // int anetTcpAccept(int serversock, char *ip, size_t ip_len, int *port) {
  //   int fd;
  //   struct sockaddr_storage sa;
  //   socklen_t salen = sizeof(sa);

  //   fd = accept4(serversock, (struct sockaddr *)&sa, &salen,
  //                SOCK_NONBLOCK | SOCK_CLOEXEC);
  //   if (fd == -1) {
  //     std::cerr << "accept: " << strerror(errno) << std::endl;
  //     return ANET_ERR;
  //   }
  //   struct sockaddr_in *s = (struct sockaddr_in *)&sa;
  //   if (ip) inet_ntop(AF_INET, (void *)&(s->sin_addr), ip, ip_len);
  //   if (port) *port = ntohs(s->sin_port);
  //   return fd;
  // }

  // client *createClient(connection *conn) {
  //   client *c = new client;

  //   /* passing NULL as conn it is possible to create a non connected client.
  //    * This is useful since all the commands needs to be executed
  //    * in the context of a client. When commands are executed in other
  //    * contexts (for instance a Lua script) we need a non connected client.
  //    */
  //   if (conn) {
  //   }
  // }

 private:
  // std::list<client> clients;

  AeEventLoop eventLoop;
  int port_;
};

int main() {
  // 创建事件循环

  TcpServer server(15001);

  // 开始事件循环
  server.Loop();
}