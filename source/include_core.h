
#pragma once

#ifndef TYON_LIB_MERGED_UNITY
    #define TYON_LIB_MERGED_UNITY 1
#endif

#ifndef TYON_ENGINE_MAIN_STANDALONE
    #define TYON_ENGINE_MAIN_STANDALONE 1
#endif


// Platform Dependencies
#include "../external/xorg_xlib/include/X11/Xlib.h"
#include "../external/xorg_xlib/include/X11/Xlib.h"
#include "../external/xorg_proto/include/X11/Xatom.h"
#include "../external/opengl/api/GL/glcorearb.h"
#include "../external/opengl/api/GL/glext.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#define module module_
// Enable Assistance with loading function pointers from hpp
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_xlib.h>
#include <vulkan/vk_enum_string_helper.h>
#undef module
#pragma clang diagnostic pop

// -- stdlib / stl --
#include "include_stl.h"

/** --Vendored Dependencies-- */
#include "include_tracy.h"

#include "../external/tachyon_lib/source/include_tachyon_lib_core.h"
#include "core.hpp"

#include "global.h"
#include "time.hpp"
#include "matrix.hpp"
#include "math.hpp"
#include "file.hpp"

// Old interface header
#include "renderer_interface.hpp"

// New headers
#include "platform_linux_x11.h"
#include "tachyon_render.h"
#include "platform_vulkan.h"

// Old stuff
#include "linux_opengl.h"
#include "linux_input_xlib.h"
#include "renderer.h"
#include "input.h"

// Experimental module support
// import platform_vulkan;
