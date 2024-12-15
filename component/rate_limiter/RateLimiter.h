#pragma once

#include <chrono>
#include <mutex>

class RateLimiter {
 public:
  virtual ~RateLimiter() = default;

  // 纯虚函数，尝试获取令牌
  virtual bool TryAcquire(int request_size) = 0;

 protected:
  std::mutex mtx_;  // 互斥锁
};