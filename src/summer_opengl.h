#ifndef WIN32_OPENGL_H

#include <gl/gl.h>

#define WGL_CONTEXT_VERSION_ARB                 0X2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define GL_ARRAY_BUFFER             0x8892
#define GL_ELEMENT_ARRAY_BUFFER     0x8893
#define GL_STATIC_DRAW              0x88E4

//shader defines
#define GL_FRAGMENT_SHADER          0x8B30
#define GL_VERTEX_SHADER            0x8B31
#define GL_COMPILE_STATUS           0x8B81
#define GL_LINK_STATUS              0x8B82
#define GL_VALIDATE_STATUS          0x8B83

typedef HGLRC type_wglCreateContextAttribsARB(HDC hdc, HGLRC hShareContext, const int *attribsList);

typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, uint32 stride, const void * pointer);
typedef void type_glDisableVertexAttribArray(GLuint index);
typedef void type_glGenBuffers(uint32 n, GLuint *buffers);
typedef void type_glBindBuffer(GLenum target, GLuint buffer);
typedef void type_glBufferData(GLenum target, uint32 sizePtr, const void * data, GLenum usage);

//shader typedefs
typedef GLuint type_glCreateShader(GLenum shaderType);
typedef void type_glShaderSource(GLuint shader, uint32 count, char **string, const GLint *length);
typedef void type_glCompileShader(GLuint shader);
typedef GLuint type_glCreateProgram();
typedef void type_glAttachShader(GLuint program, GLuint shader);
typedef void type_glLinkProgram(GLuint program);
typedef GLuint type_glGetUniformLocation(GLuint program, const char *name);
typedef void type_glUseProgram(GLuint program);
typedef void type_glUniform4f(GLint location,GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void type_glGetShaderiv(GLuint shaderId, GLenum pname, GLint* params);

#define OpenGLGlobalFunctionPtr(Name) static type_##Name * Name;
#define OPEN_GL_GET_PROC_ADDRESS(Name) (type_##Name *)wglGetProcAddress(#Name);

OpenGLGlobalFunctionPtr(wglCreateContextAttribsARB);

OpenGLGlobalFunctionPtr(glEnableVertexAttribArray);
OpenGLGlobalFunctionPtr(glVertexAttribPointer);
OpenGLGlobalFunctionPtr(glDisableVertexAttribArray);
OpenGLGlobalFunctionPtr(glGenBuffers);
OpenGLGlobalFunctionPtr(glBindBuffer);
OpenGLGlobalFunctionPtr(glBufferData);

//shader function ptrs
OpenGLGlobalFunctionPtr(glCreateShader);
OpenGLGlobalFunctionPtr(glShaderSource);
OpenGLGlobalFunctionPtr(glCompileShader);
OpenGLGlobalFunctionPtr(glCreateProgram);
OpenGLGlobalFunctionPtr(glAttachShader);
OpenGLGlobalFunctionPtr(glLinkProgram);
OpenGLGlobalFunctionPtr(glGetUniformLocation);
OpenGLGlobalFunctionPtr(glUseProgram);
OpenGLGlobalFunctionPtr(glUniform4f);
OpenGLGlobalFunctionPtr(glUniformMatrix4fv);
OpenGLGlobalFunctionPtr(glGetShaderiv);

struct ogl_shader {
	GLuint program_id;
	GLuint MVP_location;
};

static uint32 Global_Index_Buffer[QUAD_BUFFER_SIZE * 6];

static GLuint GlobalQuadBufferHandle;
static GLuint GlobalQuadIndexBufferHandle;

static GLuint WhiteTexHandle;
static GLuint RedTexHandle;
static GLuint BlueTexHandle;
static ogl_shader Global_tex_shader;
static ogl_shader Global_terrain_shader;

static float GlobalPerspectiveMatrix[4][4];

#define WIN32_OPENGL_H
#endif //WIN32_OPENGL_H