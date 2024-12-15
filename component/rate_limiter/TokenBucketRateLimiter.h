#pragma once

#include <chrono>
#include <mutex>
#include "RateLimiter.h"

class TokenBucketRateLimiter : public RateLimiter {
 public:
  TokenBucketRateLimiter(int capacity, int refill_rate)
      : capacity_(capacity), refill_rate_(refill_rate), current_tokens_(capacity) {
    last_refill_time_ = std::chrono::steady_clock::now();
  }

  bool TryAcquire(int request_size) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_refill_time_);
    current_tokens_ = std::min(
        capacity_,
        current_tokens_ + static_cast<int>(elapsed.count()) * refill_rate_);  // 更新令牌数量
    last_refill_time_ = now;  // 更新最后补充时间

    if (current_tokens_ >= request_size) {
      current_tokens_ -= request_size;  // 消耗令牌
      return true;                      // 允许请求
    } else {
      return false;  // 令牌不足，请求被拒绝
    }
  }

 private:
 private:
  int capacity_;        // 令牌桶的容量
  int refill_rate_;     // 令牌补充速率（每秒补充的令牌数量）
  int current_tokens_;  // 当前令牌数量
  std::chrono::steady_clock::time_point last_refill_time_;  // 上次补充令牌的时间
};