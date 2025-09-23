set_xmakever("2.7.3")

set_allowedmodes( "release",
                  "releasedbg",
                  "debug",
                  {default = "debug"})
-- add_requires("glslang", {configs = {binaryonly = true}})

target( "tachyon_engine" )
    set_kind( "binary" )
    set_languages( "c++20" )
    add_deps( "tachyon_shaders",
              "tracy",
              "lua_static" )

    set_toolchains( "clang" )
    set_policy("build.ccache", true)
    add_files( "source/*.cpp",
               "external/tachyon_lib/source/*.cpp"
    )

    add_includedirs( "source",
                     "external/spdlog/include/",
                     "external/tracy/public/tracy/",
                     "external/tracy/",
                     "external/lua",
                     "external/fmt/include",
                     "external/tachyon_lib/source" )

-- Speed up compilation with pre-compiled headers
    set_pcxxheader( "source/include_core.h" )
    set_pcxxheader( "source/include_stl.h" )
    set_pcxxheader( "external/tachyon_lib/source/include_tachyon_lib_core.h" )

    -- asan must be linked first
    -- add_links( "asan" )
    -- add_links( "ubsan" )

    -- Linux --
    add_links( "dl", "X11", "GL", "uuid", "vulkan" )

    add_defines( 'TRIANGULATE_PROJECT_ROOT="$(projectdir)"' )
    set_policy("check.auto_ignore_flags", false)

    if is_mode( "release" ) then
       -- set_policy("build.optimization.lto", true)
       set_optimize("faster")
    elseif is_mode ( "debug" ) then
       set_optimize( "none" )
    end
    -- Temporary cxxflags to safe the effort of full converting to xmake --
    -- Using lld linker instead of mold for now for error messages
    -- Reconsider if the build gets slow
    add_cxxflags( "clang::-g",
                  -- "-analyze",
                  -- "-fuse-ld=lld",
                  "-fuse-ld=mold",
                  "-march=native",
                  "-stdlib=libstdc++",
                  -- "-std=c++20",
                  -- "-fmodules-ts",
                  -- Generate a control flow graph
                  "gcc::-fdump-tree-all-graph",
                  -- Preprocessor Only Output (have fun finding the source files in xmake)
                  -- "-E",

                  -- Enable Tracy Profiler
                  "-DTRACY_ENABLE=1",
                  -- Can't Make this flag work
                  -- "-DTRACY_ON_DEMAND=1",

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
                  -- "clang::-Werror=old-style-cast",

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
                  -- Breaks fmtlib
                  "clang::-Wno-invalid-constexpr",
                  -- Only runs on extra semicolons that do nothing, pointless.
                  "-Wno-extra-semi-stmt",
                  -- "-fsanitize=address",
                  -- "-fsanitize=thread",
                  -- "-fsanitize=memory",
                  -- "-fsanitize=undefined",
                  -- "-fsanitize=dataflow",
                  -- "-fsanitize=cfi",   -- Control Flow Integrity
                  -- "-fsanitize=kcfi",  -- Kernel Indirect Call Forward-Edge Control Flow Integrity
                  -- "-fsanitize=safe-stack",
                  ""
                   )

target( "tracy" )
    set_kind( "static" )
    add_files( "external/tracy/public/TracyClient.cpp" )
    add_includedirs(  "external/tracy/public/tracy/",
                      "external/tracy/" )

    set_languages( "c++20" )

    set_toolchains( "clang" )

    add_links( "pthread" )
    add_cxxflags( "-DTRACY_ENABLE=1",
                  "-g",
                  "-pthread" )

target( "tachyon_libs" )
    set_kind( "static" )
    set_languages( "c++20" )
    set_default( false )


    set_toolchains( "clang" )
    set_policy("build.ccache", true)
    add_files( "source/error.cpp",
               "source/global.cpp",
               "source/file.cpp",
               "source/time.cpp" )

    add_includedirs( "external/tracy/public/tracy/",
                     "external/spdlog/include/",
                     "source",
                     "external/tachyon_lib/source" )

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
                  -- unexpected When they fail silently
                  "-Werror=unknown-pragmas",
                  -- Non-Void functions that don't return can crash *at runtime*
                  "-Werror=return-type",
                  "-Werror=inconsistent-missing-override",
                  -- Shadowing
                  "-Werror=shadow-all",
                  -- Has inconsistent behaviour and behaves like reinterpret cast, never use
                  "-Werror=old-style-cast",
                  "-Werror=empty-body",

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
       set_optimize("fastest")
    end

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

target( "tachyon_shaders" )
    set_kind( "object" )
    add_packages("glslang")
    add_rules("utils.glsl2spv", {outputdir = "build"})
    add_files("source/shaders/*.vert",
              "source/shaders/*.frag")

target( "tachyon_tests" )
    set_kind( "binary" )
    set_languages( "c++20" )
    add_deps( "tachyon_libs" )
    set_default( false )

    add_tests( "catch2" )
    add_links( "external/catch2/build/src/catch2/libCatch2.a" )
    add_files( "tests/*.cpp" )
    add_includedirs( "../tracy",
                     "external/spdlog/include/",
                     "external/catch2/src/",
                     "tests/",
                     "source" )

    add_defines( 'TRIANGULATE_PROJECT_ROOT="$(projectdir)"' )

target( "lua_static" )
    set_kind( "static" )
    add_defines( "MAKE_LIB=1" )
    add_files( "external/lua/onelua.c" )

target( "catch2" )
    set_kind( "phony" )
