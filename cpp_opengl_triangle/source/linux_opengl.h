
#pragma once

#include "renderer_interface.hpp"

#include <X11/Xlib.h>
#include <GL/glx.h>


#include <vector>
#include <array>
#include <string_view>

#include "global.h"
#include "error.hpp"


using std::unique_ptr;
using std::make_unique;
using std::array;
using namespace std::string_literals;

typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);

/** This is the Linux GL X version of the renderer
  *
  * This is hardcoded to use Xorg because OpenGL in unpopular on other platforms
  * On linux the other platform in EGL so that logic path will have to be written
  * completely seperate
 */

/// GLX Implimentation
class renderer_opengl final INTERFACE_RENDERER
{
    // Forward Declarations
    /// Helper class to store the properties of individual vertex_buffers
    struct vertex_buffer
    {
        ffloat3* data;
        fuint32 size;
        GLenum usage_pattern;
    };

    global_database* global = nullptr;

    /// Primary Display
    Display* rx_display = nullptr;
    fint32 rx_display_count = 0;
    // Multiple dispalys not yet supported
    array<Display*, 1> rx_display_list;

    /// Primary Window
    Window vx_window = 0;
    fid vx_window_id = 0;
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
    fint32 vglx_context_id = 0;

    std::vector<GLXContext> vglx_context_list;
    GLXFBConfig* vglx_fbconfigurations = nullptr;
    GLXFBConfig vglx_fbselection = nullptr;
    const char* vglx_extensions_string = nullptr;
    int vglx_fb_count = -1;
    int vglx_major = -1;
    int bglx_minor = -1;

    // OpenGL Only
    unsigned int vbo_actives[10] = {};
    unsigned int vbo = 0;
    unsigned int vao[10] = {};

    array<GLuint, 1000> mvertex_array_names;
    array<GLuint, 1000> mvertex_buffer_names;

    array<vertex_buffer, 1000>  mvertex_buffer_list;

    // Shaders
    fstring shader_fragment_source = "#version 410 core \n              \
        smooth in vec4 vertex_color;                                    \
        out vec4 frag_color; \n                                         \
        void main() \n                                                  \
        { \n                                                            \
            vec4 fc = gl_FragCoord; \n                                  \
                                                                        \
            frag_color = vertex_color;                                  \
        }\0"s;
    fstring shader_vertex_source = "#version 410 core \n                \
     layout (location = 0) in vec3 vert; \n                             \
    layout (location = 1) in vec4 col; \n                               \
    out vec4 vertex_color;                                              \
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

    fid shader_program_test = 0;
    fid shader_fragment_test = 0;
    fid shader_vertex_test = 0;

    ffloat dr = 1080.f/1920.f;  // 1080p clip space transformation
    unique_ptr<vfloat4[]> mbuffer = make_unique<vfloat4[]>( 1920*1080 );
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
    vfloat4 mtriangle_color = { .2f, .9f, .2f, 1.f };
    vfloat4 mrectangle_color = {.5f, .1f, .5f, 1.f};
    vfloat4 mcircle_color = { .2f, .9f, .2f, 1.f };
    vfloat4 msignfield_color = {1.f/255*98.f, 1.f/255*42.f, 1.f/255*125.f, 1.f};
    vfloat4 gradient_approximation[10000] = {};
    int buffer_damage_size = 1920*1080;

    CONSTRUCTOR renderer_opengl();

    GLXContext
    FUNCTION get_gl_context() const;

    fhowdit
    FUNCTION initialize() INTERFACE;

    fhowdit
    FUNCTION deinitialize() INTERFACE;

    fid
    FUNCTION display_context_create( ) INTERFACE;

    fhowdit
    FUNCTION display_context_destroy( fid target ) INTERFACE;

    fid
    FUNCTION window_create() INTERFACE;

    fhowdit
    FUNCTION window_destroy( fid target ) INTERFACE;

    fid
    FUNCTION context_create() INTERFACE;

    fhowdit
    FUNCTION context_destroy( fid target ) INTERFACE;

    fhowdit
    FUNCTION context_set_current( fid target ) INTERFACE;

    /// \type_request The OpenGL shader type to create
    fid FUNCTION shader_create( fstring_view name, shader_type type_request ) INTERFACE;

    fhowdit
    FUNCTION shader_load( fid target, fpath shader_file, bool binary = false ) INTERFACE;

    fhowdit
    FUNCTION shader_compile( fid target, fstring code ) INTERFACE;

    fid
    FUNCTION shader_program_create( fstring_view name, std::vector<fid> shaders_attach = {} ) INTERFACE;

    fhowdit
    FUNCTION shader_program_compile( fid target ) INTERFACE;

    fhowdit
    FUNCTION shader_program_attach( fid target, fid shader_attached ) INTERFACE;

    fhowdit
    FUNCTION shader_program_detach( fid target, fid shader_detached ) INTERFACE;

    fhowdit
    FUNCTION vertex_buffer_register( fid target,
                                     unique_ptr<ffloat3> backing_buffer,
                                     fuint32 buffer_size ) INTERFACE;
    
    fhowdit
    FUNCTION draw_test_triangle(vfloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION draw_test_circle(vfloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION draw_test_rectangle(vfloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION draw_test_signfield(vfloat4 p_color) INTERFACE;

    fhowdit
    FUNCTION refresh() INTERFACE;

    DESTRUCTOR ~renderer_opengl() INTERFACE;

private:
};

/// Linkage Dynamic functions with less compatability than simple functions
namespace ldynamic
{
    static PFNGLXCHOOSEFBCONFIGPROC glXChooseFBConfig;
    static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
    static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
    static PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA;

    static PFNGLBINDBUFFERPROC glBindBuffer;
    static PFNGLGENBUFFERSPROC glGenBuffers;
    static PFNGLBUFFERDATAPROC glBufferData;

    static PFNGLCREATESHADERPROC glCreateShader;
/// void glShaderSource( GLuint shader, GLsizei count, const GLchar **string, const GLint *length )
    static PFNGLSHADERSOURCEPROC glShaderSource;
    static PFNGLCOMPILESHADERPROC glCompileShader;
    static PFNGLGETSHADERIVPROC glGetShaderiv;
    static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    static PFNGLCREATEPROGRAMPROC glCreateProgram;
    static PFNGLATTACHSHADERPROC glAttachShader;
    static PFNGLATTACHSHADERPROC glDetachShader;
    static PFNGLLINKPROGRAMPROC glLinkProgram;
    static PFNGLGETPROGRAMIVPROC glGetProgramiv;
    static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    static PFNGLDELETESHADERPROC glDeleteShader;

    static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    static PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
    static PFNGLUSEPROGRAMPROC glUseProgram;
    static PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    static PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
    static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    static PFNGLMAPBUFFERPROC glMapBuffer;
    static PFNGLUNMAPBUFFERPROC glUnmapBuffer;
    static PFNGLDRAWARRAYSPROC glDrawArrays;

    static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
}


INTERFACE_IMPLEMENT_RENDERER(renderer_opengl)
