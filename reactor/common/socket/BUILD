load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "socket",
    srcs = ["socket.cc"],
    hdrs = ["socket.h"],
    visibility = ["//visibility:public"], 
    deps = [
        "//reactor/common/logger:logger",
        "//reactor/common/InetAddress:InetAddress"
        
    ],
)
