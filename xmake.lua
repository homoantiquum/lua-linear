add_rules("mode.debug", "mode.release")

add_requires("lua", "openblas")

set_warnings("all", "extra", "pedantic", "error")

target("linear")
    add_packages("openblas", "lua")
    set_kind("shared")
    set_filename("linear.dylib")

    add_files("src/*.c")
    add_defines("LINEAR_USE_AXPBY=1")
    add_ldflags("-fPIC")
