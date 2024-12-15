#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include "RateLimiter.h"

class SlidingWindowRateLimiter : public RateLimiter {
 public:
  SlidingWindowRateLimiter(int max_requests_per_win,
                           std::chrono::seconds bucket_size,
                           int num_buckets)
      : max_requests_per_win_(max_requests_per_win),
        bucket_size_(bucket_size),
        num_buckets_(num_buckets) {}

  bool TryAcquire(int request_size) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto now = std::chrono::steady_clock::now();
    // 移除所有超出窗口时间范围的桶
    while (!buckets_.empty() && now - buckets_.front().first >= num_buckets_ * bucket_size_) {
      buckets_.pop_front();
    }

    int total_requests = 0;
    for (const auto& bucket : buckets_) {
      total_requests += bucket.second;
    }

    if (total_requests + request_size <= max_requests_per_win_) {
      // 如果最后一个桶的时间戳与当前时间相同，则增加该桶的请求计数
      if (!buckets_.empty() && buckets_.back().first == now) {
        buckets_.back().second += request_size;
      } else {
        // 否则创建一个新的桶
        buckets_.push_back({now, request_size});
      }
      return true;  // 允许请求
    } else {
      return false;  // 超过最大请求数
    }
  }

 private:
  int max_requests_per_win_;          // 最大请求数
  std::chrono::seconds bucket_size_;  // 每个小桶的大小
  int num_buckets_;                   // 小桶的数量
  std::deque<std::pair<std::chrono::steady_clock::time_point, int>> buckets_;  // 桶队列
};