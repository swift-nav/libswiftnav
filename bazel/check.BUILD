load("@rules_foreign_cc//foreign_cc:defs.bzl", "cmake")

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

cmake(
    name = "check",
    cache_entries = {
        "CMAKE_C_FLAGS": "-fPIC",
    },
    lib_source = ":srcs",
    linkopts = select({
        "@bazel_tools//src/conditions:darwin": ["-lpthread"],
        "//conditions:default": ["-lpthread", "-lrt"],
    }),
    out_static_libs = select({
        "@bazel_tools//src/conditions:windows": ["check.lib"],
        "//conditions:default": ["libcheck.a"],
    }),
    visibility = ["//visibility:public"],
)
