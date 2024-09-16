// #include <arpa/inet.h>
// #include <netinet/tcp.h>
// #include <sys/socket.h>
// #include <sys/types.h>

// #include <cerrno>
// #include <cstring>
// #include <functional>
// #include <iostream>
// #include <list>
// #include <string>
// #include <system_error>

// #include "epoll/epoll.h"

// // Constants used in the code.
// static const int MAX_ACCEPTS_PER_CALL = 1000;
// static const int NET_IP_STR_LEN =
//     46;  // INET6_ADDRSTRLEN is 46 in <netinet/in.h>
// static const int ANET_ERR = -1;

// struct connection {
//   ConnectionType *type;
//   ConnectionState state;
//   short int flags;
//   short int refs;
//   int last_errno;
//   void *private_data;
//   ConnectionCallbackFunc conn_handler;
//   ConnectionCallbackFunc write_handler;
//   ConnectionCallbackFunc read_handler;
//   int fd;
// };

// typedef void (*ConnectionCallbackFunc)(struct connection *conn);

// // ConnectionType CT_Socket = {.ae_handler = connSocketEventHandler,
// //                             .close = connSocketClose,
// //                             .write = connSocketWrite,
// //                             .writev = connSocketWritev,
// //                             .read = connSocketRead,
// //                             .accept = connSocketAccept,
// //                             .connect = connSocketConnect,
// //                             .set_write_handler =
// connSocketSetWriteHandler,
// //                             .set_read_handler = connSocketSetReadHandler,
// //                             .get_last_error = connSocketGetLastError,
// //                             .blocking_connect = connSocketBlockingConnect,
// //                             .sync_write = connSocketSyncWrite,
// //                             .sync_read = connSocketSyncRead,
// //                             .sync_readline = connSocketSyncReadLine,
// //                             .get_type = connSocketGetType};
// typedef struct ConnectionType {
//   void (*ae_handler)(struct aeEventLoop *el, int fd, void *clientData,
//                      int mask);
//   int (*connect)(struct connection *conn, const char *addr, int port,
//                  const char *source_addr,
//                  ConnectionCallbackFunc connect_handler);
//   int (*write)(struct connection *conn, const void *data, size_t data_len);
//   int (*writev)(struct connection *conn, const struct iovec *iov, int
//   iovcnt); int (*read)(struct connection *conn, void *buf, size_t buf_len);
//   void (*close)(struct connection *conn);
//   int (*accept)(struct connection *conn, ConnectionCallbackFunc
//   accept_handler); int (*set_write_handler)(struct connection *conn,
//                            ConnectionCallbackFunc handler, int barrier);
//   int (*set_read_handler)(struct connection *conn,
//                           ConnectionCallbackFunc handler);
//   const char *(*get_last_error)(struct connection *conn);
//   int (*blocking_connect)(struct connection *conn, const char *addr, int
//   port,
//                           long long timeout);
//   ssize_t (*sync_write)(struct connection *conn, char *ptr, ssize_t size,
//                         long long timeout);
//   ssize_t (*sync_read)(struct connection *conn, char *ptr, ssize_t size,
//                        long long timeout);
//   ssize_t (*sync_readline)(struct connection *conn, char *ptr, ssize_t size,
//                            long long timeout);
//   int (*get_type)(struct connection *conn);
// } ConnectionType;

// typedef enum {
//   CONN_STATE_NONE = 0,
//   CONN_STATE_CONNECTING,
//   CONN_STATE_ACCEPTING,
//   CONN_STATE_CONNECTED,
//   CONN_STATE_CLOSED,
//   CONN_STATE_ERROR
// } ConnectionState;

// class client {};

// class TcpServer {
//  public:
//   TcpServer(int port) : eventLoop(1000), port_(port) {

//     if (eventLoop.aeCreateFileEvent(
//             0, AE_READABLE,
//             std::bind(acceptTcpHandler, this, std::placeholders::_1,
//                       std::placeholders::_2, std::placeholders::_3),
//             NULL) == AE_ERR) {
//       return;
//     }
//   };

//   int Loop() { eventLoop.aeMain(); }

//  private:
//   void acceptTcpHandler(int fd, void *privdata, int mask) {
//     std::string cip;
//     cip.resize(NET_IP_STR_LEN);
//     int cport;
//     int maxAccepts = MAX_ACCEPTS_PER_CALL;
//     while (maxAccepts--) {
//       int cfd = anetTcpAccept(fd, &cip[0], cip.size(), &cport);
//       if (cfd == ANET_ERR) {
//         if (errno != EWOULDBLOCK) {
//           std::cerr << "Accepting client connection err" << std::endl;
//         }
//         return;
//       }
//       // Truncate the string to the actual length of the IP string.
//       cip.resize(std::strlen(cip.c_str()));
//       std::cout << "Accepted " << cip << ":" << cport << std::endl;

//       // connCreateAcceptedSocket
//       connection *conn = new connection;
//       //   conn->type = &CT_Socket;  // TODO
//       conn->fd = fd;
//       conn->state = CONN_STATE_ACCEPTING;

//       // acceptCommonHandler
//       client *c;
//       char conninfo[100];
//       /* Limit the number of connections we take at the same time.
//        *
//        * Admission control will happen before a client is created and
//        * connAccept() called, because we don't want to even start
//        * transport-level negotiation if rejected. */

//       /* Create connection and client */
//       client *c = new client;
//     }
//   }

//   /* Accept a connection and also make sure the socket is non-blocking, and
//    * CLOEXEC. returns the new socket FD, or -1 on error. */
//   int anetTcpAccept(int serversock, char *ip, size_t ip_len, int *port) {
//     int fd;
//     struct sockaddr_storage sa;
//     socklen_t salen = sizeof(sa);

//     fd = accept4(serversock, (struct sockaddr *)&sa, &salen,
//                  SOCK_NONBLOCK | SOCK_CLOEXEC);
//     if (fd == -1) {
//       std::cerr << "accept: " << strerror(errno) << std::endl;
//       return ANET_ERR;
//     }
//     struct sockaddr_in *s = (struct sockaddr_in *)&sa;
//     if (ip) inet_ntop(AF_INET, (void *)&(s->sin_addr), ip, ip_len);
//     if (port) *port = ntohs(s->sin_port);
//     return fd;
//   }

//   client *createClient(connection *conn) {
//     client *c = new client;

//     /* passing NULL as conn it is possible to create a non connected client.
//      * This is useful since all the commands needs to be executed
//      * in the context of a client. When commands are executed in other
//      * contexts (for instance a Lua script) we need a non connected client.
//      */
//     if (conn) {
//     }
//   }

//  private:
//   std::list<client> clients;

//   AeEventLoop eventLoop;
//   int port_;
// };

// int main() {
//   // 创建事件循环

//   TcpServer server(15001);

//   // 开始事件循环
//   server.Loop();
// }