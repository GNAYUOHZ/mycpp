
#include "common/InetAddress/InetAddress.h"
#include "tcpserver.h"

int main() {
  reactor::InetAddress listenAddr(9981);
  reactor::TcpServer server(listenAddr);
  server.start();
}