
#pragma once

// Platform Dependencies
#include <X11/Xlib.h>
#include <X11/Xatom.h>

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

struct fmesh;

#include <tachyon_code_helpers.h>
#include "core.hpp"

#include "include_tachyon_lib_core.h"
using namespace tyon;

#include "global.h"
#include "table.h"
#include "memory.hpp"
#include "string.hpp"
#include "error.hpp"
#include "time.hpp"
#include "matrix.hpp"
#include "math.hpp"
#include "extensions.hpp"
#include "file.hpp"

#include "renderer_interface.hpp"
#include "platform_linux_x11.h"
#include "platform_vulkan.h"
#include "linux_opengl.h"
#include "linux_input_xlib.h"
#include "tachyon_render.h"
#include "renderer.h"
#include "input.h"

// Experimental module support
// import platform_vulkan;
