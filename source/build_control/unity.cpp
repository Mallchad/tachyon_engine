
#include "../include_core.h"
#include "../error.cpp"
#include "../file.cpp"
#include "../global.cpp"
#include "../input.cpp"
#include "../linux_input_xlib.cpp"
#include "../linux_opengl.cpp"
#include "../math.cpp"
#include "../memory.cpp"
#include "../platform_linux_x11.cpp"
#include "../platform_vulkan.cpp"
#include "../tachyon_render.cpp"
#include "../renderer.cpp"
#include "../string.cpp"
#include "../time.cpp"

#ifndef TYON_LIB_MERGED_UNITY
    #define TYON_LIB_MERGED_UNITY 1
#endif

#ifndef TYON_ENGINE_MAIN_STANDALONE
    #define TYON_ENGINE_MAIN_STANDALONE 1
#endif

#if (TYON_LIB_MERGED_UNITY)
    #include "..//external/tachyon_lib/source/build_control/tachyon_lib_unity_core.cpp"
#endif TYON_ENGINE_UNITY_BUILD

// #include "external/lua/onelua.c"

#if (TYON_ENGINE_MAIN_STANDALONE)
    // Main include
    #include "../main.cpp"
#endif TYON_ENGINE_UNITY_BUILD
