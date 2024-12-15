load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "buffer",
    srcs = ["buffer.cc"],
    hdrs = ["buffer.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "buffer_test",
    srcs = ["buffer_test.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "buffer",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)
