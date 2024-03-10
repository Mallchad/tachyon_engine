
#pragma once

#include "renderer_interface.hpp"
#include "include_core.h"

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <memory>
#include <vector>
#include <array>

#include "global.h"
#include "error.hpp"

using std::unique_ptr;
using std::make_unique;
using std::array;

namespace std
{
    template<class _t> class initializer_list;
}
using coord3 = vector3<GLfloat>;
using srgba  = vector4<GLfloat>;
using rgba   = vector4<GLfloat>;

using PFNGLDRAWARRAYSPROC = void (APIENTRYP)(GLenum mode, GLint first, GLsizei count);


/** This is the Linux GL X version of the renderer
  *
  * This is hardcoded to use Xorg because OpenGL in unpopular on other platforms
  * On linux the other platform in EGL so that logic path will have to be written
  * completely seperate
 */

/// GLX Implimentation
class renderer_opengl final INTERFACE_RENDERER
{
    friend class input_xlib;
    // Forward Declarations
    struct mesh_metadata
    {
        /// Identifying label for the mesh
        fstring name = "UNLABELLED_mesh";

        /// Backend handle for managing this object
        mesh_id reference_id = -1;

        /// Backhend handle for managing a vertex array that stores attributes
        // describing how buffers associated with vertecies but not necessarily
        // actual vertex data is laid out
        attribute_id vertex_attribute_id = -1;

        /// Backend handle for managing a vertex list and GL arary object
        buffer_id vertex_buffer_id = -1;

        /// Backend handle for managing a index list and GL array object
        buffer_id vertex_index_buffer_id = -1;

        /// Backend handle for managing a vertex color list and GL array object
        buffer_id vertex_color_buffer_id = -1;

        /// OpenGL specific access and write pattern, use GL_STATIC_DRAW if unsure
        GLenum usage_pattern = GL_STATIC_DRAW;

    };
    using fmesh_metadata = mesh_metadata;

    /// Helper class to store the properties of individual vertex_buffers
    struct vertex_buffer
    {
        ffloat3* data;
        fuint32 size;
        GLenum usage_pattern;
    };

    globals* global = nullptr;

    /// Primary Display
    Display* rx_display = nullptr;
    fint32 rx_display_count = 0;
    // Multiple dispalys not yet supported
    array<Display*, 1> rx_display_list;

    /// Primary Window
    Window vx_window = 0;
    window_id vx_window_id = 0;
    std::vector<Window> vx_window_list = { 0 };

    fint32 vx_default_screen = -1;
    fint32 display_width = -1;
    fint32 display_height = -1;
    int vx_connection_number = -1;
    char vx_connection_string[20] = {};

    XVisualInfo* vx_buffer_config = nullptr;
    XSetWindowAttributes vx_window_attributes = {};
    std::vector<Atom> vx_window_protocols = {};
    Atom vx_wm_delete_window = 0;
    Atom wm_state = 0;
    Atom wm_state_fullscreen = 0;
    Atom wm_state_maximized_vert = 0;
    Atom wm_state_maximized_horz = 0;
    Atom wm_state_above = 0;
    Atom wm_allowed_actions = 0;

    // GL X extensions
    // Priamry Thread Local Context
    GLXContext vglx_context = nullptr;
    context_id vglx_context_id = 0;

    std::vector<GLXContext> vglx_context_list;
    GLXFBConfig* vglx_fbconfigurations = nullptr;
    GLXFBConfig vglx_fbselection = nullptr;
    const char* vglx_extensions_string = nullptr;
    int vglx_fb_count = -1;

    fint32 m_gl_extension_count = -1;
    fstring m_gl_extension_string;

    // Minimum version required for function
    int vglx_major = 3;
    int vglx_minor = 3;

    static constexpr fuint32 mbuffer_limit = 1000;
    static constexpr fuint32 mattribute_limit = 1000;
    static constexpr fuint32 mmesh_limit = 1000;

    array<GLuint, mattribute_limit>      mattribute_names;
    array<GLuint, mbuffer_limit>                mbuffer_names;
    array<vertex_buffer, mbuffer_limit>         mbuffer_list;
    array<fmesh, mmesh_limit>                   mmesh_list;
    array<fmesh_metadata, mmesh_limit>          mmesh_metadata_list;

    fint32 mattribute_count = 0;
    fint32 mbuffer_count = 0;
    fint32 mmesh_count = 0;

    buffer_id uniform_frame_globals = 0;

    // Shaders
    fstring shader_fragment_source = "#version 330 core \n              \
        smooth in vec4 vertex_color;                                    \
        out vec4 frag_color; \n                                         \
        void main() \n                                                  \
        { \n                                                            \
            vec4 fc = gl_FragCoord; \n                                  \
                                                                        \
            frag_color = vertex_color;                                  \
        }\0"s;
    fstring shader_vertex_source = "#version 330 core \n                \
     layout (location = 0) in vec3 vert; \n                             \
    layout (location = 2) in vec4 col; \n                               \
    smooth out vec4 vertex_color;                                       \
    void main() \n                                                      \
    { \n                                                                \
        gl_Position = vec4(vert.x, vert.y, vert.z, 1.f); \n             \
                                                         vertex_color = col; \
    }\0"s;                                                              \

    static constexpr fint32 shader_limit = 1000;
    array<GLuint, shader_limit> shader_list;
    array<char[30], shader_limit> shader_names;
    fuint32 shader_count = 0;

    static constexpr fint32 shader_program_limt = 1000;
    array<GLuint, shader_program_limt> shader_program_list;
    array<fstring, shader_program_limt> shader_program_names;
    fuint32 shader_program_count = 0;

    shader_program_id shader_program_test = 0;
    shader_id shader_fragment_test = 0;
    shader_id shader_vertex_test = 0;

    ffloat dr = 1080.f/1920.f;  // 1080p clip space transformation
    unique_ptr<rgba[]> mbuffer = make_unique<rgba[]>( 1920*1080 );

    mesh_id mtest_triangle_mesh = -1;
    // 0.433 pre-computed magic number for unit triangle
    float mtest_triangle[9] =
    {
        -0.5f * dr, -0.4330127019f, 0.0f,
        0.5f * dr, -0.4330127019f, 0.0f,
        0.0f * dr,  0.4330127019f, 0.0f
    };
    float mtest_triangle_colors[12] =
    {
        1.f, .0f, .0f, 0.f,
        .0f, 1.f, .0f, 0.f,
        .0f, .0f, 1.f, 0.f
    };

    int progress_x = 0;
    int progress_y = 0;

public:
    ffloat4 mtriangle_color = { .2f, .9f, .2f, 1.f };
    ffloat4 mrectangle_color = {.5f, .1f, .5f, 1.f};
    ffloat4 mcircle_color = { .2f, .9f, .2f, 1.f };
    ffloat4 msignfield_color = {1.f/255*98.f, 1.f/255*42.f, 1.f/255*125.f, 1.f};
    ffloat4 gradient_approximation[10000] = {};
    int buffer_damage_size = 1920*1080;

    CONSTRUCTOR renderer_opengl();

    GLXContext
    FUNCTION get_gl_context() const;

    fhowdit
    FUNCTION initialize() INTERFACE;

    fhowdit
    FUNCTION deinitialize() INTERFACE;

    display_id
    FUNCTION display_context_create() INTERFACE;

    fhowdit
    FUNCTION display_context_destroy( display_id target ) INTERFACE;

    window_id
    FUNCTION window_create() INTERFACE;

    fhowdit
    FUNCTION window_destroy( window_id target ) INTERFACE;

    context_id
    FUNCTION context_create() INTERFACE;

    fhowdit
    FUNCTION context_destroy( context_id target ) INTERFACE;

    fhowdit
    FUNCTION context_set_current( context_id target ) INTERFACE;

    /// \type_request The OpenGL shader type to create
    shader_id
    FUNCTION shader_create( fstring name, shader_type type_request ) INTERFACE;

    fhowdit
    FUNCTION shader_load( shader_id target, fpath shader_file, bool binary = false ) INTERFACE;

    fhowdit
    FUNCTION shader_compile( shader_id target, fstring code ) INTERFACE;

    shader_program_id
    FUNCTION shader_program_create( fstring name,
                                    std::initializer_list<shader_id> shaders_attach = {}) INTERFACE;

    fhowdit
    FUNCTION shader_program_destroy( shader_program_id target );

    fhowdit
    FUNCTION shader_program_compile( shader_program_id target ) INTERFACE;

    fhowdit
    FUNCTION shader_program_attach( shader_program_id target, shader_id shader_attached ) INTERFACE;

    fhowdit
    FUNCTION shader_program_detach( shader_program_id target, shader_id shader_detached ) INTERFACE;

    fhowdit
    FUNCTION shader_program_run( shader_program_id target ) INTERFACE;

    freport
    FUNCTION shader_globals_update( frame_shader_global contents );

    /// Register a mesh object with the backend and copy buffer data
    mesh_id
    FUNCTION mesh_create( fmesh target ) INTERFACE;

    fhowdit
    FUNCTION draw_mesh( mesh_id target,
                        ftransform target_transform,
                        shader_program_id target_shader ) INTERFACE;

    fhowdit
    FUNCTION draw_test_triangle(ffloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION draw_test_circle(ffloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION draw_test_rectangle(ffloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION draw_test_signfield(ffloat4 p_color) INTERFACE;

    freport
    FUNCTION frame_start();

    fhowdit
    FUNCTION refresh( frame_shader_global ) INTERFACE;

    DESTRUCTOR ~renderer_opengl() INTERFACE;

private:
    // Returns the OpenGL internal id for the associated buffer
    GLuint
    FUNCTION get_buffer( buffer_id target );
};
