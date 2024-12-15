#include <gtest/gtest.h>
#include <thread>
#include "FixedWindowRateLimiter.h"
#include "LeakyBucketRateLimiter.h"
#include "SlidingWindowRateLimiter.h"
#include "TokenBucketRateLimiter.h"

// TokenBucketRateLimiter Tests
TEST(TokenBucketRateLimiterTest, AcquireWithinCapacity) {
  TokenBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(5), true);
  EXPECT_EQ(limiter.TryAcquire(3), true);
}

TEST(TokenBucketRateLimiterTest, AcquireExceedsCapacity) {
  TokenBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(10), true);
  EXPECT_EQ(limiter.TryAcquire(1), false);
}

TEST(TokenBucketRateLimiterTest, RefillMultipleTokens) {
  TokenBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(10), true);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  EXPECT_EQ(limiter.TryAcquire(4), true);
}

TEST(TokenBucketRateLimiterTest, RefillDoesNotExceedCapacity) {
  TokenBucketRateLimiter limiter(10, 2);
  std::this_thread::sleep_for(std::chrono::seconds(6));
  EXPECT_EQ(limiter.TryAcquire(10), true);
  EXPECT_EQ(limiter.TryAcquire(1), false);
}

// FixedWindowRateLimiter Tests
TEST(FixedWindowRateLimiterTest, AcquireWithinLimit) {
  FixedWindowRateLimiter limiter(10, std::chrono::seconds(1));
  EXPECT_EQ(limiter.TryAcquire(5), true);
  EXPECT_EQ(limiter.TryAcquire(3), true);
}

TEST(FixedWindowRateLimiterTest, AcquireExceedsLimit) {
  FixedWindowRateLimiter limiter(10, std::chrono::seconds(1));
  EXPECT_EQ(limiter.TryAcquire(10), true);
  EXPECT_EQ(limiter.TryAcquire(1), false);
}

TEST(FixedWindowRateLimiterTest, ResetAfterWindow) {
  FixedWindowRateLimiter limiter(10, std::chrono::seconds(1));
  EXPECT_EQ(limiter.TryAcquire(10), true);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(limiter.TryAcquire(5), true);
}

// SlidingWindowRateLimiter Tests
TEST(SlidingWindowRateLimiterTest, AcquireWithinLimit) {
  SlidingWindowRateLimiter limiter(10, std::chrono::seconds(1), 10);
  EXPECT_EQ(limiter.TryAcquire(5), true);
  EXPECT_EQ(limiter.TryAcquire(3), true);
}

TEST(SlidingWindowRateLimiterTest, AcquireExceedsLimit) {
  SlidingWindowRateLimiter limiter(10, std::chrono::seconds(1), 10);
  EXPECT_EQ(limiter.TryAcquire(10), true);
  EXPECT_EQ(limiter.TryAcquire(1), false);
}

TEST(SlidingWindowRateLimiterTest, BucketsExpire) {
  SlidingWindowRateLimiter limiter(2, std::chrono::seconds(1), 2);
  EXPECT_EQ(limiter.TryAcquire(2), true);
  std::this_thread::sleep_for(std::chrono::seconds(3));
  EXPECT_EQ(limiter.TryAcquire(2), true);
}

// LeakyBucketRateLimiter Tests
TEST(LeakyBucketRateLimiterTest, AcquireWithinCapacity) {
  LeakyBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(5), true);
  EXPECT_EQ(limiter.TryAcquire(3), true);
}

TEST(LeakyBucketRateLimiterTest, AcquireExceedsCapacity) {
  LeakyBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(10), true);
  EXPECT_EQ(limiter.TryAcquire(1), false);
}

TEST(LeakyBucketRateLimiterTest, LeakTokens) {
  LeakyBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(10), true);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(limiter.TryAcquire(1), true);
}

TEST(LeakyBucketRateLimiterTest, LeakMultipleTokens) {
  LeakyBucketRateLimiter limiter(10, 2);
  EXPECT_EQ(limiter.TryAcquire(10), true);               // 请求 10 个令牌，成功
  std::this_thread::sleep_for(std::chrono::seconds(2));  // 等待 2 秒
  EXPECT_EQ(limiter.TryAcquire(4), true);  // 请求 4 个令牌，成功（令牌已泄漏）
}

TEST(LeakyBucketRateLimiterTest, LeakDoesNotUnderflow) {
  LeakyBucketRateLimiter limiter(10, 2);
  std::this_thread::sleep_for(std::chrono::seconds(6));  // 等待 6 秒
  EXPECT_EQ(limiter.TryAcquire(1), true);  // 请求 1 个令牌，成功（令牌已泄漏到 0）
}