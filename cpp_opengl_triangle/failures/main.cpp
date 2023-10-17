
#include <cstdlib>
#include <memory>
#include <iostream>

// Intentional Libraries
// #include <glad/glad.h>
#include <GL/glew.h>
// #include <GLFW/glfw3.h>
// OpenGL function prototypes
// #include <GL/glext.h>
#include <GL/glxew.h>
// #include <GL/glxext.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxproto.h>

// Xorg
#include <X11/Xlib.h>
#include <X11/Xlibint.h>

// Standard Lbirary
#include <string>
#include <bits/unique_ptr.h>

bool g_continue_main = true;

typedef void (*FGL_VIEWPORT) (GLsizei, GLuint*);
// extern FGL_VIEWPORT glViewport = nullptr;

void window_resize_callback(GLFWwindow* window, int width, int height)
{
    // glViewport(50, 50, width ? width : 100 , height ? height : 100);
    // glViewport(100, 100, 100, 100);
}

int xerror_handler(Display *dsp, XErrorEvent *error)
{
    char errorstring[128];
    XGetErrorText(dsp, error->error_code, errorstring, 128);

    std::cout << "ack!fatal: X error--" << errorstring << std::endl;
}

int main() try
{
    using std::unique_ptr;

    Display* x_connection = nullptr;
    XVisualInfo* x_buffer_config = nullptr;
    int glx_framebuffer_option_count = 0;
    std::basic_string<GLubyte> glx_context_request = reinterpret_cast<const GLubyte*>("glXCreateContextARB");
    PFNGLXCREATECONTEXTATTRIBSARBPROC glxCreateContextARB = nullptr;
    GLXFBConfig glx_fbconfig = nullptr;
    GLXContext glx_context = nullptr;

    // Initialization and error checking

    x_connection = XOpenDisplay(nullptr);
    if (x_connection == nullptr)
    {
        std::cout << "Failed to open X display" << std::endl;
        return EXIT_FAILURE;
    }

    XSetErrorHandler(xerror_handler);

    // glxCreateContextARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress(glx_context_request.c_str());
    static int visual_attribs[] =
        {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_RED_SIZE        , 8,
            GLX_GREEN_SIZE      , 8,
            GLX_BLUE_SIZE       , 8,
            GLX_ALPHA_SIZE      , 8,
            GLX_DEPTH_SIZE      , 24,
            GLX_STENCIL_SIZE    , 8,
            GLX_DOUBLEBUFFER    , True,
            //GLX_SAMPLE_BUFFERS  , 1,
            //GLX_SAMPLES         , 4,
            None
        };
  if (glxCreateContextARB == nullptr)
  {
      std::cout << "Failed to get glxCreateContextARB function" << std::endl;
  }

  // glx_fbconfig =;
  glx_context = glxCreateContextARB(x_connection, glx_fbconfig, nullptr, false, nullptr);

  if (glx_context == nullptr)
  {
      std::cout << "Failed to create glx context" << std::endl;
      int glx_error = -1;
      std::string error_description = "";
      switch (glx_error)
      {
      case GLXBadContext:
          error_description = "GLXBadContext";
      case GLXBadContextState:
          error_description = "GLXBadContextState";
      case GLXBadCurrentDrawable:
          error_description = "GLXBadCurrentDrawable";
      case GLXBadCurrentWindow:
          error_description = "GLXBadCurrentWindow";
      case GLXBadDrawable:
          error_description = "GLXBadDrawable";
      case GLXBadFBConfig:
          error_description = "GLXBadFBConfig";
      }
      std::cout << "GLX threw the error, descrion: " << error_description << std::endl;
      return EXIT_FAILURE;
  }

  GLenum error_glew = glewInit();

  glfwInit();
  GLFWwindow* window_context = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  if (window_context == nullptr)
  {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return EXIT_FAILURE;
  }

  if(error_glew != GLEW_OK)
  {
      // Problem: glewInit failed, something is seriously wrong.
      std::cout << "glewInit failed: " << glewGetErrorString(error_glew) << std::endl;
      return EXIT_FAILURE;
  }

// glViewport = (FGL_VIEWPORT) glXGetProcAddress("glViewport");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwMakeContextCurrent(window_context);
// if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
// {
//     std::cout << "Failed to initialize GLAD" << std::endl;
//     return EXIT_FAILURE;
// }
// glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window_context, window_resize_callback);

// glfwSetFramebufferSizeCallback(window_context, window_resize_callback);

  unsigned int VBO;
// glGenBuffers(1, &VBO);

  while(!glfwWindowShouldClose(window_context) && g_continue_main)
                                                                                                          {
                                                                                                              glfwSwapBuffers(window_context);
                                                                                                              glfwPollEvents();
                                                                                                          }

// Cleanup
                                                                                                          glfwTerminate();
    return EXIT_SUCCESS;
}

catch (...)
{
    glfwTerminate();
    std::cerr << "An exception occured in main, exiting" << std::endl;
    return EXIT_FAILURE;
}
