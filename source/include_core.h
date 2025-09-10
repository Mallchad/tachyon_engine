
#pragma once

// -- stdlib / stl --
#include <array>
#include <compare>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

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
#include "linux_opengl.h"
#include "linux_input_xlib.h"
#include "renderer.h"
#include "input.h"
