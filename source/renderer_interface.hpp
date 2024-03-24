
#pragma once

#include "include_core.h"

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
#endif

/// Underlying Display Server Handle, like an X11 'Display' or a GDI Handle
using display_id        = internal_id<id_type::display>;
/// Individual Created Window
using window_id         = internal_id<id_type::window>;
/// Graphical Context
using context_id        = internal_id<id_type::graphics_context>;
/// Underlying Graphics Vertex Attribute, mostly OpenGL specific
using attribute_id      = internal_id<id_type::vertex_attribute>;
/// Underlying Graphics API Buffer, backed by internal copy
using buffer_id         = internal_id<id_type::buffer>;
/// Internal Mesh Format
using mesh_id           = internal_id<id_type::mesh>;
// Single Stage Shader
using shader_id         = internal_id<id_type::shader>;
/// Shader Program
using shader_program_id = internal_id<id_type::shader_program>;
/// Shader Program
using program_id        = internal_id<id_type::shader_program>;

struct frame_shader_global
{
    // Timestamp of the very beginning of the program lifetime
    ffloat epoch = 0;
    // Time elapsed since program epoch
    ffloat time_since_epoch = 0;
    /// Time since epoch at the beginning of previous frame
    ffloat last_begin_epoch = 0;
    /// Time since epoch at end of previous frame
    ffloat last_end_epoch = 0;
    /// Time between last frame and current frame measured at unspecified time during frame
    ffloat delta_time = 0;
    /// Time between last frame and current frame measured at beginning of each frame
    ffloat delta_time_begin = 0;
    /// Time between last frame and current frame measured at beginning of each frame
    ffloat delta_time_end = 0;
    // Screen aspect ratio given as vertical over horizontal
    ffloat screen_vh_aspect_ratio = 1080.f/1920.f;
};

enum class shader_type
{
    vertex,
    fragment,
    geometry,
    compute,
    tesselation_control,
    tesselation_eval
};

/// All attributes are baked when copied to the graphiccs layer
// be mindful of this
struct mesh
{
    fstring name;
    std::vector<ffloat3> vertex_buffer;
    std::vector<fuint32> vertex_index_buffer;
    std::vector<ffloat4> vertex_color_buffer;
    fuint32 face_count = 0;
    fuint32 vertex_count = 0;
    fuint32 index_count = 0;
    fuint32 color_count = 0;
    shader_program_id shader_id = -1;
};

using fshader_type = shader_type;
using fmesh = mesh;

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

    virtual display_id
    FUNCTION display_context_create( ) PURE;

    virtual fhowdit
    FUNCTION display_context_destroy( display_id target ) PURE;

    virtual window_id
    FUNCTION window_create() PURE;

    virtual fhowdit
    FUNCTION window_destroy( window_id target ) PURE;

    /// Create the context for the relevant platform and return an id
    virtual context_id
    FUNCTION context_create() PURE;

    virtual fhowdit
    FUNCTION context_destroy( context_id target ) PURE;

    /// Set the context to the context ID
    virtual fhowdit
    FUNCTION context_set_current( context_id target ) PURE;

    /// Register a new shader object and return an ID
    virtual shader_id
    FUNCTION shader_create( fstring name, shader_type type_request ) PURE;

    /** Attempt to load a shader program from disk, either code or a compiled binary.
     * This might may be an intermediate compile format or a platform specific
     * pre-compiled or cached shader
     *
     * This is intentionally seperate to manage the performance penality of
     * loading things from disk */
    virtual fhowdit
    FUNCTION shader_load( shader_id target, fpath shader_file, bool binary = false ) PURE;

    /** Attempt to compile the provided shader to a native-loadable code.
     * This is intentionally seperate to management the performance penaltiy
     * of compiling shaders at runtime */
    virtual fhowdit
    FUNCTION shader_compile( shader_id target, fstring code ) PURE;

    /// \shaders_attach attach the listed shader ID's if presence
    virtual shader_program_id
    FUNCTION shader_program_create( fstring name,
                                    std::initializer_list<shader_id> shaders_attach ) PURE;

    virtual fhowdit
    FUNCTION shader_program_compile( shader_program_id target ) PURE;

    virtual fhowdit
    FUNCTION shader_program_attach( shader_program_id target, shader_id shader_attached ) PURE;

    virtual fhowdit
    FUNCTION shader_program_detach( shader_program_id target, shader_id shader_detached ) PURE;

    virtual fhowdit
    FUNCTION shader_program_run( shader_program_id target ) PURE;

    virtual mesh_id
    FUNCTION mesh_create( fmesh mesh ) PURE;

    virtual fhowdit
    FUNCTION draw_mesh( mesh_id target,
                        ftransform target_transform,
                        shader_program_id target_shader ) PURE;

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
