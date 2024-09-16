
#include <sys/timerfd.h>
#include <unistd.h>
#include <chrono>
#include "InetAddress.h"
#include "acceptor.h"
#include "channel.h"
#include "eventloop.h"
#include "logger.h"
#include "tcpserver.h"

void timeout(const char* msg) { LOG_INFO << "timeout " << msg; }

void onConnection(const reactor::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    printf("onConnection() succ: new connection [%s] from %s\n",
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());

    ::sleep(5);
    std::string message1(10000, 'A');
    conn->send(message1);
  } else {
    printf("onConnection(): connection [%s] is down\n", conn->name().c_str());
  }
}

void onMessage(const reactor::TcpConnectionPtr& conn, reactor::Buffer* buf, int64_t receiveTime) {
  printf("onMessage(): received %zd bytes from connection [%s] at %ld, content: %s\n",
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime,
         buf->retrieveAllAsString().c_str());
}

void onWriteComplete(const reactor::TcpConnectionPtr& conn) { printf("onWriteComplete()\n"); }

int main() {
  reactor::EventLoop eventloop;

  // test timerfd
  // int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
  //                   std::chrono::system_clock::now().time_since_epoch())
  //                   .count();
  // eventloop.runAfter(1000, std::bind(timeout, "once"));
  // eventloop.runEvery(2000, std::bind(timeout, "every"));
  // eventloop.runAt(now + 3000, std::bind(timeout, "at"));

  // test
  // // nc localhost 9981
  // reactor::InetAddress listenAddr(9981);
  // reactor::Acceptor acceptor(&eventloop, listenAddr);
  // acceptor.setNewConnectionCallback(newConnection);
  // acceptor.listen();

  reactor::InetAddress listenAddr(9981);
  reactor::TcpServer server(&eventloop, listenAddr);
  server.setConnectionCallback(onConnection);
  server.setMessageCallback(onMessage);
  server.setWriteCompleteCallback(onWriteComplete);
  server.start();

  eventloop.loop();
}