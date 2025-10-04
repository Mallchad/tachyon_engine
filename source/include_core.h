
#pragma once

// Platform Dependencies
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define module module_
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#undef module

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
#include "renderer.h"
#include "input.h"

// Experimental module support
// import platform_vulkan;
