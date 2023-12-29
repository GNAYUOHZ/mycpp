// g++ main.cc epoll.cc --std=c++17
#include <unistd.h>

#include <iostream>

#include "epoll.h"

// 文件事件回调函数
void fileEventCallback(int fd, void *clientData, int mask) {
  char buffer[64];
  ssize_t bytesRead;

  bytesRead = read(fd, buffer, sizeof(buffer));
  if (bytesRead > 0) {
    write(1, buffer, bytesRead);  // 输出到stdout
  }
}

// 事件循环休眠前的回调
void beforeSleepProc() { std::cout << "beforeSleepProc" << std::endl; }

// 事件循环休眠后的回调
void afterSleepProc() { std::cout << "afterSleepProc" << std::endl; }

int main() {
  // 创建事件循环，设定大小为 10
  AeEventLoop eventLoop(10);

  // 设置事件循环的回调
  eventLoop.aeSetBeforeSleepProc(beforeSleepProc);
  eventLoop.aeSetAfterSleepProc(afterSleepProc);
  // eventLoop.aeSetDontWait(0);

  // 注册标准输入的文件事件
  if (eventLoop.aeCreateFileEvent(STDIN_FILENO, AE_READABLE, fileEventCallback,
                                  NULL) == AE_ERR)
    return AE_ERR;

  // eventLoop.aeDeleteFileEvent(STDIN_FILENO, AE_READABLE);

  // 开始事件循环
  // eventLoop.aeStop();
  eventLoop.aeMain();

  return 0;
}
