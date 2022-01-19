CMAKE_FALSE_CONSTANTS = ["0", "OFF", "NO", "FALSE", "N", "IGNORE", "NOTFOUND"]

def _configure_file_impl(ctx):
    vars = {}
    for (key, val) in ctx.attr.vars.items():
        cmake_define = "#cmakedefine {}".format(key)
        define = "// #undef {}".format(key) if val in CMAKE_FALSE_CONSTANTS else "#define {}".format(key)

        vars[cmake_define] = define
        vars["@{}@".format(key)] = val
        vars["${" + key + "}"] = val

    out = ctx.actions.declare_file(ctx.attr.out)
    ctx.actions.expand_template(
        output = out,
        template = ctx.file.template,
        substitutions = vars,
    )
    return [DefaultInfo(files = depset([out]))]

configure_file = rule(
    implementation = _configure_file_impl,
    attrs = {
        "vars": attr.string_dict(),
        "out": attr.string(),
        "template": attr.label(
            allow_single_file = [".in"],
            mandatory = True,
        ),
    },
)
