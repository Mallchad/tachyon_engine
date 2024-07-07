set_xmakever("2.7.3")

set_defaultmode( "release" )
add_requires("glslang", {configs = {binaryonly = true}})

target( "tachyon_engine" )
    set_kind( "binary" )
    set_languages( "c++20" )
    add_packages("glslang")
    set_policy("build.optimization.lto", true)

    set_toolchains( "clang" )
    set_policy("build.ccache", true)
    add_files( "source/*.cpp" )
    add_includedirs( "../tracy",
                     "external/spdlog/include/",
                     "source" )

    -- asan must be linked first
    -- add_links( "asan" )
    -- add_links( "ubsan" )
    add_links( "dl", "X11", "GL" )
    add_defines( 'TRIANGULATE_PROJECT_ROOT="$(projectdir)"' )
    set_policy("check.auto_ignore_flags", false)
    -- Temporary cxxflags to safe the effort of full converting to xmake --
    -- Using lld linker instead of mold for now for error messages
    -- Reconsider if the build gets slow
    add_cxxflags( "clang::-g",
                  -- "-analyze",
                  -- "-fuse-ld=lld",
                  "-fuse-ld=mold",
                  -- Generate a control flow graph
                  "gcc::-fdump-tree-all-graph",

                  -- Performance Flags
                  -- "-msse",

                  -- Warnings
                  "clang::-Wpedantic",
                  "-Wall",
                  -- Whitelist Errors
                  -- #import is a Obj-C language extension and easily confused with C++ modules
                  "clang::-Werror=import-preprocessor-directive-pedantic",
                  -- Platform unknown pragmas lead to some insanely annoying and
                  -- unexpected When they fail silentlys
                  "clang::-Werror=unknown-pragmas",
                  -- Non-Void functions that don't return can crash *at runtime*
                  "clang::-Werror=return-type",
                  "clang::-Werror=inconsistent-missing-override",
                  -- Shadowing
                  "clang::-Werror=shadow-all",
                  -- Has inconsistent behaviour and behaves like reinterpret cast, never use
                  "clang::-Werror=old-style-cast",

                  -- -- Disable Warnings
                  "clang::-Wno-unused-value",
                  "clang::-Wno-padded",
                  "clang::-Wno-c++98-compat",
                  "clang::-Wno-c++98-compat-pedantic ",
                  "clang::-Wno-documentation-unknown-command ",
                  "clang::-Wno-unreachable-code-break",
                  "clang::-Wno-unused-variable",
                  "clang::-Wno-unused-private-field",
                  "clang::-Wno-abstract-final-class",
                  -- Only runs on extra semicolons that do nothing, pointless.
                  "-Wno-extra-semi-stmt"
                  -- "-fsanitize=address",
                  -- "-fsanitize=thread",
                  -- "-fsanitize=memory",
                  -- "-fsanitize=undefined",
                  -- "-fsanitize=dataflow",
                  -- "-fsanitize=cfi",   -- Control Flow Integrity
                  -- "-fsanitize=kcfi",  -- Kernel Indirect Call Forward-Edge Control Flow Integrity
                  -- "-fsanitize=safe-stack",

                   )

target( "tachyon_libs" )
    set_kind( "static" )
    set_languages( "c++20" )
    add_packages("glslang")

    set_toolchains( "clang" )
    set_policy("build.ccache", true)
    add_files( "source/error.cpp",
               "source/global.cpp",
               "source/file.cpp",
               "source/time.cpp" )

    add_includedirs( "../tracy",
                     "external/spdlog/include/",
                     "source" )

    -- asan must be linked first
    -- add_links( "asan" )
    -- add_links( "ubsan" )
    add_links( "dl", "X11", "GL" )
    add_defines( 'TRIANGULATE_PROJECT_ROOT="$(projectdir)"' )

    -- Temporary cxxflags to safe the effort of full converting to xmake --
    -- Using lld linker instead of mold for now for error messages
    -- Reconsider if the build gets slow
    add_cxxflags( "clang::-g",
                  -- "-fuse-ld=lld",
                  "-fuse-ld=mold",

                  -- Warnings
                  "-Wpedantic",
                  "-Wall",
                  -- Whitelist Errors
                  -- #import is a Obj-C language extension and easily confused with C++ modules
                  "-Werror=import-preprocessor-directive-pedantic",
                  -- Platform unknown pragmas lead to some insanely annoying and
                  -- unexpected When they fail silentlys
                  "-Werror=unknown-pragmas",
                  -- Non-Void functions that don't return can crash *at runtime*
                  -- "-Werror=return-type",
                  "-Werror=inconsistent-missing-override",
                  -- Shadowing
                  "-Werror=shadow-all",
                  -- Has inconsistent behaviour and behaves like reinterpret cast, never use
                  "-Werror=old-style-cast",

                  -- -- Disable Warnings
                  "-Wno-unused-value",
                  "-Wno-padded",
                  "-Wno-c++98-compat",
                  "-Wno-c++98-compat-pedantic ",
                  "-Wno-documentation-unknown-command ",
                  "-Wno-unreachable-code-break",
                  "-Wno-unused-variable",
                  "-Wno-unused-private-field",
                  "-Wno-abstract-final-class",
                  -- Only runs on extra semicolons that do nothing, pointless.
                  "-Wno-extra-semi-stmt"
                  -- "-fsanitize=address",
                  -- "-fsanitize=thread",
                  -- "-fsanitize=memory",
                  -- "-fsanitize=undefined",
                  -- "-fsanitize=dataflow",
                  -- "-fsanitize=cfi",   -- Control Flow Integrity
                  -- "-fsanitize=kcfi",  -- Kernel Indirect Call Forward-Edge Control Flow Integrity
                  -- "-fsanitize=safe-stack",

                   )

    if is_mode( "release" ) then
       add_cxxflags( "clang::-g",
                     "-Weverything", "-Wno-implicit-int-float-conversion",
                     "-Wno-sign-conversion",
                     "-Wpedantic",
                     "-Wall",
                     "-Wno-unused-value",
                     "-Wno-padded",
                     "-Werror=return-type",
                     "-Werror=inconsistent-missing-override",
                     "-Wno-c++98-compat",
                     "-Wno-c++98-compat-pedantic",
                     "-Wno-documentation-unknown-command",
                     "-Wno-unreachable-code-break",
                     "-Werror=shadow" )
    end

target( "shaders" )
    set_kind( "phony" )
    add_rules("utils.glsl2spv", {outputdir = "build"})
    add_files("source/shaders/*.vert",
              "source/shaders/*.frag")

target( "tachyon_tests" )
    set_kind( "binary" )
    set_languages( "c++20" )
    add_deps( "tachyon_libs" )

    add_tests( "catch2" )
    add_links( "build/linux/x86_64/release/libtachyon_libs.a",
               "external/catch2/build/src/catch2/libCatch2.a" )
    add_files( "tests/*.cpp" )
    add_includedirs( "../tracy",
                     "external/spdlog/include/",
                     "external/catch2/src/",
                     "tests/",
                     "source" )

    add_defines( 'TRIANGULATE_PROJECT_ROOT="$(projectdir)"' )

target( "catch2" )
    set_kind( "phony" )
