
#pragma once

// Passed through to platform specific renderer
#include <type_traits>
#include <bits/unique_ptr.h>
#include <string_view>
#include <vector>

#include "code_helpers.h"
#include "error.hpp"
#include "math.hpp"

/** To check for implimented functions place the INTERFACE_DEFINE_FUNCTION macro
 * at the bottom of the .h file. It will not work properly if it is put in a
 * .cpp file, although it will work it will just throw undefined references and
 * linker error which defeats the point of the macro
 *
 */

#define INTERFACE_DEFINE_FUNCTION(interface_function_name)      \
    static_assert(1);                                                     \
    // static_assert( std::is_member_function_pointer<             \
    //                decltype( &interface_function_name )>()); \

// Use derived class as a debug-only virtual interface
#ifdef DEBUG_INTERFACE
    // Use INTERFACE macro after interface defined function to help validate it
    #define INTERFACE override

    #define INTERFACE_IMPLEMENT_RENDERER(...)
    #define INTERFACE_RENDERER \
        : i_renderer

// Fall back to primitive error system
#else
    #define INTERFACE

    #define INTERFACE_RENDERER
    #define INTERFACE_IMPLEMENT_RENDERER(interface_class_name)                \
                                                                            \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::create_context)       \
                                                                            \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::vertex_buffer_register) \
                                                                            \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_triangle)   \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_circle)     \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_rectangle)  \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::draw_test_signfield)  \
        INTERFACE_DEFINE_FUNCTION(interface_class_name::refresh)        \

#endif

using std::unique_ptr;
using std::make_unique;

enum class shader_type
{
    vertex,
    fragment,
    geometry,
    compute,
    tesselation_control,
    tesselation_eval
};

struct mesh
{
    fstring_view name;
    ffloat3* vertex_buffer = nullptr;
    fuint32* vertex_index_buffer = nullptr;
    ffloat4* vertex_color_buffer = nullptr;
    fuint32 vertex_count = 0;
    fuint32 index_count = 0;
    fuint32 color_count = 0;
    fid shader_program_id;
};

typedef shader_type fshader_type;
typedef mesh fmesh;

#ifdef DEBUG_INTERFACE
/// Interface for Platform Specific Renderer Layer
class i_renderer
{
protected:

public:
    virtual fhowdit
    FUNCTION initialize() PURE;

    virtual fhowdit
    FUNCTION deinitialize() PURE;

    virtual fid
    FUNCTION display_context_create( ) PURE;

    virtual fhowdit
    FUNCTION display_context_destroy( fid target ) PURE;

    virtual fid
    FUNCTION window_create() PURE;

    virtual fhowdit
    FUNCTION window_destroy( fid target ) PURE;

    /// Create the context for the relevant platform and return an id
    virtual fid
    FUNCTION context_create() PURE;

    virtual fhowdit
    FUNCTION context_destroy( fid target ) PURE;

    /// Set the context to the context ID
    virtual fhowdit
    FUNCTION context_set_current( fid target ) PURE;

    /// Register a new shader object and return an ID
    virtual fid
    FUNCTION shader_create( fstring_view name, shader_type type_request ) PURE;

    /** Attempt to load a shader program from disk, either code or a compiled binary.
     * This might may be an intermediate compile format or a platform specific
     * pre-compiled or cached shader
     *
     * This is intentionally seperate to manage the performance penality of
     * loading things from disk */
    virtual fhowdit
    FUNCTION shader_load( fid target, fpath shader_file, bool binary = false ) PURE;

    /** Attempt to compile the provided shader to a native-loadable code.
     * This is intentionally seperate to management the performance penaltiy
     * of compiling shaders at runtime */
    virtual fhowdit
    FUNCTION shader_compile( fid target, fstring code ) PURE;

    /// \shaders_attach attach the listed shader ID's if presence
    virtual fid
    FUNCTION shader_program_create( fstring_view name, std::vector<fid> shaders_attach ) PURE;

    virtual fhowdit
    FUNCTION shader_program_compile( fid target ) PURE;

    virtual fhowdit
    FUNCTION shader_program_attach( fid target, fid shader_attached ) PURE;

    virtual fhowdit
    FUNCTION shader_program_detach( fid target, fid shader_detached ) PURE;

    virtual fhowdit
    FUNCTION shader_program_run( fid target ) PURE;

    virtual fid
    FUNCTION mesh_create( fmesh mesh ) PURE;

    virtual fhowdit
    FUNCTION draw_mesh( fid target, ftransform target_transform, fid target_shader ) PURE;

    virtual fhowdit
    FUNCTION draw_test_triangle( ffloat4 p_color ) PURE;

    virtual fhowdit
    FUNCTION draw_test_circle( ffloat4 p_color ) PURE;

    virtual fhowdit
    FUNCTION draw_test_rectangle( ffloat4 p_color ) PURE;

    /// Draw a a wide gradient that goes to opposing colour around a defined circle
    virtual fhowdit
    FUNCTION draw_test_signfield( ffloat4 p_color ) PURE;

    /// Attempt to refresh the display and window contexts
    virtual fhowdit
    FUNCTION refresh() PURE;

    virtual
    DESTRUCTOR ~i_renderer() {}
};
#endif
