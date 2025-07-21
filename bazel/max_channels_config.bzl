load("@rules_swiftnav//tools:configure_file.bzl", "configure_file_impl")
load("@bazel_skylib//rules:common_settings.bzl", "BuildSettingInfo")

def _max_channels_config_impl(ctx):
    vars = {
        "MAX_CHANNELS": str(ctx.attr.max_channels[BuildSettingInfo].value),
    }
    return configure_file_impl(ctx, vars)

max_channels_config = rule(
    implementation = _max_channels_config_impl,
    attrs = {
        "out": attr.string(),
        "template": attr.label(
            allow_single_file = [".in"],
            mandatory = True,
        ),
        "max_channels": attr.label(),
    },
)
