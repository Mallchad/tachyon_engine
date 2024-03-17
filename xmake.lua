set_xmakever("2.7.3")

set_defaultmode( "release" )

target( "triangulite" )
    set_kind( "binary" )
    set_languages( "c++20" )

    set_toolchains( "clang" )
    set_policy("build.ccache", true)
    add_files( "source/*.cpp" )
    add_includedirs( "../tracy",
                     "external/spdlog/include/",
                     "source" )
    -- asan must be linked first
    add_links( "asan" )
    add_links( "ubsan" )
    add_links( "dl", "X11", "GL" )
    add_defines( 'TRIANGULATE_PROJECT_ROOT="$(projectdir)"' )

    -- Temporary cxxflags to safe the effort of full converting to xmake --
    -- Using lld linker instead of mold for now for error messages
    -- Reconsider if the build gets slow
    add_cxxflags( "clang::-g",
                  "-Wpedantic",
                  "-Wall",
                  "-Wno-unused-value",
                  "-Wno-padded",
                  "-Werror=return-type",
                  "-Werror=inconsistent-missing-override",
                  "-Wno-c++98-compat",
                  "-Wno-c++98-compat-pedantic ",
                  "-Wno-documentation-unknown-command ",
                  "-Wno-unreachable-code-break",
                  "-Werror=shadow ",
                  "-fuse-ld=lld",
                  "-fmodules-ts ",
                  "-Wno-unused-variable",
                  "-Wno-unused-private-field",
                  "-Wno-abstract-final-class",
                  "-fsanitize=address",
                  -- "-fsanitize=thread",
                  -- "-fsanitize=memory",
                  "-fsanitize=undefined",
                  -- "-fsanitize=dataflow",
                  -- "-fsanitize=cfi",   -- Control Flow Integrity
                  -- "-fsanitize=kcfi",  -- Kernel Indirect Call Forward-Edge Control Flow Integrity
                  -- "-fsanitize=safe-stack",
                  "-Werror=old-style-cast" )

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
                     "-Werror=shadow -std=c++20" )
    end
