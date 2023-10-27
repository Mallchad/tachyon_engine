
#pragma once


#include <GL/glx.h>

class global_database
{
public:
    static global_database* primary_database;

    static global_database* get_primary();
    bool kill_program = false;
};

static PFNGLXCHOOSEFBCONFIGPROC _glXChooseFBConfig;
static PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
static PFNGLXSWAPINTERVALMESAPROC glXSwapIntervalMESA;

static PFNGLBINDBUFFERPROC glBindBuffer;
static PFNGLGENBUFFERSPROC glGenBuffers;
static PFNGLBUFFERDATAPROC glBufferData;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
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
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
static PFNGLDRAWARRAYSPROC intern_glDrawArrays;

static PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
