
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
/// Uniform Management Object
using uniform_id        = internal_id<id_type::uniform>;
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

using alignment_format = std::array<fint32, 30>;

class uniform
{
    /// How many members in the instance of t_struct
    int member_count = 0;
    int size = 0;
    uniform_id id;
    buffer_id id_buffer;
    alignment_format format;
    /// Copy stage for copying to a uniform in the correct format
    byte_buffer copy_buffer;

public:
    constexpr fbyte*
    FUNCTION data()
    {
        return copy_buffer.data();
    }

    /// Packs data from a struct into a staging buffer ready to be copied into a uniform
    template<typename ...t_shaders>
    constexpr freport
    FUNCTION pack( t_shaders... member_list )
    {
        fint32 argument_count = sizeof...(t_shaders);
        bool setup_alignment = false;

        // Setup pack format if it doesn't already exist
        if (member_count <= 0)
        {
            member_count = argument_count;
            setup_alignment = true;
        }
        if ( argument_count != member_count )
        {
            print( "Provided member count does not match previously provided pack definition" );
            return false;
        }
        member_count = argument_count;
        fint32 iterations = 0;

        // Meta-Programming Loop
        FOLD([=]( auto new_member,
                  alignment_format& out_format,
                  fint32& out_size,
                  byte_buffer& out_copy_buffer,
                  fint32& i_member ) -> void
        {
            using member_t = decltype(new_member);
            constexpr bool float_type = std::is_floating_point_v< member_t >;
            constexpr bool int_type = std::is_integral_v< member_t >;
            constexpr bool vector_type = (std::is_same_v< member_t, ffloat3> ||
                                          std::is_same_v< member_t, ffloat4>);
            constexpr bool matrix_type = false;
            static_assert( float_type || int_type || vector_type || matrix_type,
                           "Uniform can only contain shader types: float, int, vector, matrix" );
            if (setup_alignment)
            {
                // Each data type has a specific alignment associated with it
                constexpr fint32 alignment_size = ((float_type || int_type) ? 4 :
                                                   vector_type || matrix_type ? 16 :
                                                   -1);
                const fint32 alignment_multiple = cast<fint32>(
                    std::ceil( out_size / alignment_size ));
                fint32 alignment_location = ( alignment_multiple * alignment_size );
                out_size = alignment_location + alignment_size;
                out_format[ i_member ] = alignment_location;
                out_copy_buffer.resize( out_size );
            }

            const fint32 member_size = sizeof(new_member);
            const fint32 offset = out_format[ i_member ];
            std::memcpy( offset+ out_copy_buffer.data(), &new_member, member_size );
            ++i_member;
        }( member_list, format, size, copy_buffer, iterations ), ...);

        return false;
    }

};
