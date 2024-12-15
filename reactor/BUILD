load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "eventloop",
    srcs = ["eventloop.cc","channel.cc","timerqueue.cc"
   ,"tcpserver.cc","tcpconnection.cc"],
    hdrs = ["eventloop.h","channel.h","timerqueue.h",
   "tcpserver.h","tcpconnection.h"],
    visibility = ["//visibility:public"], 
    deps = [
        "//reactor/common/logger:logger",
        "//reactor/common/buffer:buffer",
        "//reactor/common/InetAddress:InetAddress",
        "//reactor/common/socket:socket"
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    visibility = ["//visibility:public"], 
    deps = [
        "//reactor:eventloop",
        "//reactor/common/logger:logger",
        "//reactor/common/InetAddress:InetAddress"
    ],
)
