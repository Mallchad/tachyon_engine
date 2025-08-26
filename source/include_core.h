
#pragma once

// -- stdlib / stl --
#include <array>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include <compare>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <type_traits>

#include <csignal>

/** --Vendored Dependencies-- */
#include "include_tracy.h"

struct fmesh;

#include <tachyon_code_helpers.h>
#include "core.hpp"

#include "include_tachyon_lib_core.h"
using namespace vmec;
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
#include "linux_opengl.h"
#include "linux_input_xlib.h"
#include "renderer.h"
#include "input.h"
