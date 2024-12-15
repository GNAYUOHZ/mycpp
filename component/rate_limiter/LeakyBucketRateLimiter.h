#pragma once

#include <chrono>
#include <mutex>
#include "RateLimiter.h"

class LeakyBucketRateLimiter : public RateLimiter {
 public:
  LeakyBucketRateLimiter(int capacity, int leak_rate)
      : capacity_(capacity), leak_rate_(leak_rate), current_water_(0) {
    last_leak_time_ = std::chrono::steady_clock::now();
  }

  bool TryAcquire(int request_size) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_leak_time_);
    int leaked_water = static_cast<int>(elapsed.count()) * leak_rate_;
    if (leaked_water > 0) {
      current_water_ = std::max(0, current_water_ - leaked_water);  // 减少水量
      last_leak_time_ = now;                                        // 更新最后漏水时间
    }

    if (current_water_ + request_size <= capacity_) {
      current_water_ += request_size;  // 将请求放入漏桶
      return true;                     // 允许请求
    } else {
      return false;  // 漏桶已满，请求被拒绝
    }
  }

 private:
  int capacity_;                                          // 漏桶的容量
  int leak_rate_;                                         // 漏水速率（每秒漏掉的水量）
  int current_water_;                                     // 当前水量
  std::chrono::steady_clock::time_point last_leak_time_;  // 上次漏水的时间
};