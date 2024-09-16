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
        "//reactor:buffer",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "logger",
    srcs = ["logger.cc"],
    hdrs = ["logger.h"],
    visibility = ["//visibility:public"], 
)

cc_library(
    name = "eventloop",
    srcs = ["eventloop.cc","channel.cc","poller.cc","timerqueue.cc"
    ,"acceptor.cc","InetAddress.cc","socket.cc","tcpserver.cc","tcpconnection.cc","buffer.cc"],
    hdrs = ["eventloop.h","channel.h","poller.h","timerqueue.h","timer.h",
    "acceptor.h","InetAddress.h","socket.h","tcpserver.h","tcpconnection.h","buffer.h"],
    visibility = ["//visibility:public"], 
    deps = [
        "//reactor:logger",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    visibility = ["//visibility:public"], 
    deps = [
        "//reactor:eventloop",
        "//reactor:logger",
    ],
)
