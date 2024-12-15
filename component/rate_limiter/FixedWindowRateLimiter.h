#pragma once

#include <chrono>
#include <mutex>
#include "RateLimiter.h"

class FixedWindowRateLimiter : public RateLimiter {
 public:
  FixedWindowRateLimiter(int max_requests_per_win, std::chrono::seconds window_size)
      : max_requests_per_win_(max_requests_per_win), window_size_(window_size), request_count_(0) {
    window_start_time_ = std::chrono::steady_clock::now();
  }

  bool TryAcquire(int request_size) override {
    std::lock_guard<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();
    // 如果当前时间在窗口内
    if (now - window_start_time_ < window_size_) {
      // 检查请求数量是否超过限制
      if (request_count_ + request_size <= max_requests_per_win_) {
        request_count_ += request_size;  // 增加请求计数
        return true;                     // 允许请求
      } else {
        return false;  // 超过最大请求数
      }
    } else {  // 重置窗口
      window_start_time_ = now;
      request_count_ = request_size;  // 重置请求计数为当前请求数量
      return true;                    // 允许请求
    }
  }

 private:
  int max_requests_per_win_;                                 // 最大请求数
  std::chrono::seconds window_size_;                         // 窗口大小
  int request_count_;                                        // 当前请求计数
  std::chrono::steady_clock::time_point window_start_time_;  // 窗口开始时间
};