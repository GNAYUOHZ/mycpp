load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "rate_limiter",
    hdrs = [
        "TokenBucketRateLimiter.h",
        "FixedWindowRateLimiter.h",
        "SlidingWindowRateLimiter.h",
        "LeakyBucketRateLimiter.h",
        "RateLimiter.h",
    ],
    includes = ["."],
    deps = [
        "@com_google_googletest//:gtest_main",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "rate_limiter_test",
    srcs = ["RateLimiterTest.cc"],
    deps = [
        ":rate_limiter",
        "@com_google_googletest//:gtest_main",
    ],
)
