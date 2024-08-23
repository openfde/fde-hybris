
#define GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>

#define HYBRIS_LIBNAME "libGLESv2.so.2"  // soname
#define HYBRIS_ENVNAME "HYBRIS-GLESv2"
#include "binding.h"

HYBRIS_IMPLEMENT_FUNCTION1(void, glActiveTexture, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glAttachShader, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glBindAttribLocation, GLuint, GLuint,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindBuffer, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindFramebuffer, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindRenderbuffer, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindTexture, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBlendColor, GLfloat, GLfloat, GLfloat,
                           GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glBlendEquation, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBlendEquationSeparate, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBlendFunc, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBlendFuncSeparate, GLenum, GLenum, GLenum,
                           GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBufferData, GLenum, GLsizeiptr, const void *,
                           GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBufferSubData, GLenum, GLintptr, GLsizeiptr,
                           const void *);
HYBRIS_IMPLEMENT_FUNCTION1(GLenum, glCheckFramebufferStatus, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClear, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION4(void, glClearColor, GLfloat, GLfloat, GLfloat,
                           GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearDepthf, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearStencil, GLint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColorMask, GLboolean, GLboolean, GLboolean,
                           GLboolean);
HYBRIS_IMPLEMENT_FUNCTION1(void, glCompileShader, GLuint);
HYBRIS_IMPLEMENT_FUNCTION8(void, glCompressedTexImage2D, GLenum, GLint, GLenum,
                           GLsizei, GLsizei, GLint, GLsizei, const void *);
HYBRIS_IMPLEMENT_FUNCTION9(void, glCompressedTexSubImage2D, GLenum, GLint,
                           GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei,
                           const void *);
HYBRIS_IMPLEMENT_FUNCTION8(void, glCopyTexImage2D, GLenum, GLint, GLenum, GLint,
                           GLint, GLsizei, GLsizei, GLint);
HYBRIS_IMPLEMENT_FUNCTION8(void, glCopyTexSubImage2D, GLenum, GLint, GLint,
                           GLint, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION0(GLuint, glCreateProgram);
HYBRIS_IMPLEMENT_FUNCTION1(GLuint, glCreateShader, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glCullFace, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteBuffers, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteFramebuffers, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDeleteProgram, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteRenderbuffers, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDeleteShader, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteTextures, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDepthFunc, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDepthMask, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDepthRangef, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDetachShader, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDisable, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDisableVertexAttribArray, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glDrawArrays, GLenum, GLint, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION4(void, glDrawElements, GLenum, GLsizei, GLenum,
                           const void *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEnable, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEnableVertexAttribArray, GLuint);
HYBRIS_IMPLEMENT_FUNCTION0(void, glFinish);
HYBRIS_IMPLEMENT_FUNCTION0(void, glFlush);
HYBRIS_IMPLEMENT_FUNCTION4(void, glFramebufferRenderbuffer, GLenum, GLenum,
                           GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glFramebufferTexture2D, GLenum, GLenum, GLenum,
                           GLuint, GLint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glFrontFace, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenBuffers, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glGenerateMipmap, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenFramebuffers, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenRenderbuffers, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenTextures, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION7(void, glGetActiveAttrib, GLuint, GLuint, GLsizei,
                           GLsizei *, GLint *, GLenum *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION7(void, glGetActiveUniform, GLuint, GLuint, GLsizei,
                           GLsizei *, GLint *, GLenum *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetAttachedShaders, GLuint, GLsizei,
                           GLsizei *, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(GLint, glGetAttribLocation, GLuint, const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetBooleanv, GLenum, GLboolean *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetBufferParameteriv, GLenum, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION0(GLenum, glGetError);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetFloatv, GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetFramebufferAttachmentParameteriv, GLenum,
                           GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetIntegerv, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetProgramiv, GLuint, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetProgramInfoLog, GLuint, GLsizei,
                           GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetRenderbufferParameteriv, GLenum, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetShaderiv, GLuint, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetShaderInfoLog, GLuint, GLsizei, GLsizei *,
                           GLchar *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetShaderPrecisionFormat, GLenum, GLenum,
                           GLint *, GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetShaderSource, GLuint, GLsizei, GLsizei *,
                           GLchar *);
HYBRIS_IMPLEMENT_FUNCTION1(const GLubyte *, glGetString, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameterfv, GLenum, GLenum,
                           GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameteriv, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetUniformfv, GLuint, GLint, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetUniformiv, GLuint, GLint, GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(GLint, glGetUniformLocation, GLuint, const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetVertexAttribfv, GLuint, GLenum,
                           GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetVertexAttribiv, GLuint, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetVertexAttribPointerv, GLuint, GLenum,
                           void **);
HYBRIS_IMPLEMENT_FUNCTION2(void, glHint, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsBuffer, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsEnabled, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsFramebuffer, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsProgram, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsRenderbuffer, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsShader, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsTexture, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLineWidth, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLinkProgram, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPixelStorei, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPolygonOffset, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION7(void, glReadPixels, GLint, GLint, GLsizei, GLsizei,
                           GLenum, GLenum, void *);
HYBRIS_IMPLEMENT_FUNCTION0(void, glReleaseShaderCompiler);
HYBRIS_IMPLEMENT_FUNCTION4(void, glRenderbufferStorage, GLenum, GLenum, GLsizei,
                           GLsizei);
HYBRIS_IMPLEMENT_FUNCTION2(void, glSampleCoverage, GLfloat, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION4(void, glScissor, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glShaderBinary, GLsizei, const GLuint *,
                           GLenum, const void *, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION4(void, glShaderSource, GLuint, GLsizei,
                           const GLchar *const *, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glStencilFunc, GLenum, GLint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glStencilFuncSeparate, GLenum, GLenum, GLint,
                           GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glStencilMask, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glStencilMaskSeparate, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glStencilOp, GLenum, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glStencilOpSeparate, GLenum, GLenum, GLenum,
                           GLenum);
HYBRIS_IMPLEMENT_FUNCTION9(void, glTexImage2D, GLenum, GLint, GLint, GLsizei,
                           GLsizei, GLint, GLenum, GLenum, const void *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterf, GLenum, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterfv, GLenum, GLenum,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameteri, GLenum, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameteriv, GLenum, GLenum,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION9(void, glTexSubImage2D, GLenum, GLint, GLint, GLint,
                           GLsizei, GLsizei, GLenum, GLenum, const void *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glUniform1f, GLint, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform1fv, GLint, GLsizei, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glUniform1i, GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform1iv, GLint, GLsizei, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform2f, GLint, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform2fv, GLint, GLsizei, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform2i, GLint, GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform2iv, GLint, GLsizei, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniform3f, GLint, GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform3fv, GLint, GLsizei, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniform3i, GLint, GLint, GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform3iv, GLint, GLsizei, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glUniform4f, GLint, GLfloat, GLfloat, GLfloat,
                           GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform4fv, GLint, GLsizei, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glUniform4i, GLint, GLint, GLint, GLint,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform4iv, GLint, GLsizei, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix2fv, GLint, GLsizei, GLboolean,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix3fv, GLint, GLsizei, GLboolean,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix4fv, GLint, GLsizei, GLboolean,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glUseProgram, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glValidateProgram, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttrib1f, GLuint, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttrib1fv, GLuint, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glVertexAttrib2f, GLuint, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttrib2fv, GLuint, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glVertexAttrib3f, GLuint, GLfloat, GLfloat,
                           GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttrib3fv, GLuint, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glVertexAttrib4f, GLuint, GLfloat, GLfloat,
                           GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttrib4fv, GLuint, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION6(void, glVertexAttribPointer, GLuint, GLint, GLenum,
                           GLboolean, GLsizei, const void *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glViewport, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION1(void, glReadBuffer, GLenum);
HYBRIS_IMPLEMENT_FUNCTION6(void, glDrawRangeElements, GLenum, GLuint, GLuint,
                           GLsizei, GLenum, const void *);
HYBRIS_IMPLEMENT_FUNCTION10(void, glTexImage3D, GLenum, GLint, GLint, GLsizei,
                            GLsizei, GLsizei, GLint, GLenum, GLenum,
                            const void *);
HYBRIS_IMPLEMENT_FUNCTION11(void, glTexSubImage3D, GLenum, GLint, GLint, GLint,
                            GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum,
                            const void *);
HYBRIS_IMPLEMENT_FUNCTION9(void, glCopyTexSubImage3D, GLenum, GLint, GLint,
                           GLint, GLint, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION9(void, glCompressedTexImage3D, GLenum, GLint, GLenum,
                           GLsizei, GLsizei, GLsizei, GLint, GLsizei,
                           const void *);
HYBRIS_IMPLEMENT_FUNCTION11(void, glCompressedTexSubImage3D, GLenum, GLint,
                            GLint, GLint, GLint, GLsizei, GLsizei, GLsizei,
                            GLenum, GLsizei, const void *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenQueries, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteQueries, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsQuery, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBeginQuery, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEndQuery, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetQueryiv, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetQueryObjectuiv, GLuint, GLenum, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glUnmapBuffer, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetBufferPointerv, GLenum, GLenum, void **);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDrawBuffers, GLsizei, const GLenum *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix2x3fv, GLint, GLsizei,
                           GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix3x2fv, GLint, GLsizei,
                           GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix2x4fv, GLint, GLsizei,
                           GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix4x2fv, GLint, GLsizei,
                           GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix3x4fv, GLint, GLsizei,
                           GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniformMatrix4x3fv, GLint, GLsizei,
                           GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION10(void, glBlitFramebuffer, GLint, GLint, GLint, GLint,
                            GLint, GLint, GLint, GLint, GLbitfield, GLenum);
HYBRIS_IMPLEMENT_FUNCTION5(void, glRenderbufferStorageMultisample, GLenum,
                           GLsizei, GLenum, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glFramebufferTextureLayer, GLenum, GLenum,
                           GLuint, GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION4(void *, glMapBufferRange, GLenum, GLintptr,
                           GLsizeiptr, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION3(void, glFlushMappedBufferRange, GLenum, GLintptr,
                           GLsizeiptr);
HYBRIS_IMPLEMENT_FUNCTION1(void, glBindVertexArray, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteVertexArrays, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenVertexArrays, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsVertexArray, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetIntegeri_v, GLenum, GLuint, GLint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glBeginTransformFeedback, GLenum);
HYBRIS_IMPLEMENT_FUNCTION0(void, glEndTransformFeedback);
HYBRIS_IMPLEMENT_FUNCTION5(void, glBindBufferRange, GLenum, GLuint, GLuint,
                           GLintptr, GLsizeiptr);
HYBRIS_IMPLEMENT_FUNCTION3(void, glBindBufferBase, GLenum, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glTransformFeedbackVaryings, GLuint, GLsizei,
                           const GLchar *const *, GLenum);
HYBRIS_IMPLEMENT_FUNCTION7(void, glGetTransformFeedbackVarying, GLuint, GLuint,
                           GLsizei, GLsizei *, GLsizei *, GLenum *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glVertexAttribIPointer, GLuint, GLint, GLenum,
                           GLsizei, const void *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetVertexAttribIiv, GLuint, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetVertexAttribIuiv, GLuint, GLenum,
                           GLuint *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glVertexAttribI4i, GLuint, GLint, GLint, GLint,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glVertexAttribI4ui, GLuint, GLuint, GLuint,
                           GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttribI4iv, GLuint, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttribI4uiv, GLuint, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetUniformuiv, GLuint, GLint, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(GLint, glGetFragDataLocation, GLuint,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glUniform1ui, GLint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform2ui, GLint, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glUniform3ui, GLint, GLuint, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glUniform4ui, GLint, GLuint, GLuint, GLuint,
                           GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform1uiv, GLint, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform2uiv, GLint, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform3uiv, GLint, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniform4uiv, GLint, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glClearBufferiv, GLenum, GLint, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glClearBufferuiv, GLenum, GLint,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glClearBufferfv, GLenum, GLint,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glClearBufferfi, GLenum, GLint, GLfloat,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION2(const GLubyte *, glGetStringi, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glCopyBufferSubData, GLenum, GLenum, GLintptr,
                           GLintptr, GLsizeiptr);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetUniformIndices, GLuint, GLsizei,
                           const GLchar *const *, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glGetActiveUniformsiv, GLuint, GLsizei,
                           const GLuint *, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(GLuint, glGetUniformBlockIndex, GLuint,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetActiveUniformBlockiv, GLuint, GLuint,
                           GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glGetActiveUniformBlockName, GLuint, GLuint,
                           GLsizei, GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUniformBlockBinding, GLuint, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glDrawArraysInstanced, GLenum, GLint, GLsizei,
                           GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glDrawElementsInstanced, GLenum, GLsizei,
                           GLenum, const void *, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION2(GLsync, glFenceSync, GLenum, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsSync, GLsync);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDeleteSync, GLsync);
HYBRIS_IMPLEMENT_FUNCTION3(GLenum, glClientWaitSync, GLsync, GLbitfield,
                           GLuint64);
HYBRIS_IMPLEMENT_FUNCTION3(void, glWaitSync, GLsync, GLbitfield, GLuint64);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetInteger64v, GLenum, GLint64 *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glGetSynciv, GLsync, GLenum, GLsizei,
                           GLsizei *, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetInteger64i_v, GLenum, GLuint, GLint64 *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetBufferParameteri64v, GLenum, GLenum,
                           GLint64 *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenSamplers, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteSamplers, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsSampler, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindSampler, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glSamplerParameteri, GLuint, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glSamplerParameteriv, GLuint, GLenum,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glSamplerParameterf, GLuint, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glSamplerParameterfv, GLuint, GLenum,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetSamplerParameteriv, GLuint, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetSamplerParameterfv, GLuint, GLenum,
                           GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttribDivisor, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindTransformFeedback, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteTransformFeedbacks, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenTransformFeedbacks, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsTransformFeedback, GLuint);
HYBRIS_IMPLEMENT_FUNCTION0(void, glPauseTransformFeedback);
HYBRIS_IMPLEMENT_FUNCTION0(void, glResumeTransformFeedback);
HYBRIS_IMPLEMENT_FUNCTION5(void, glGetProgramBinary, GLuint, GLsizei, GLsizei *,
                           GLenum *, void *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramBinary, GLuint, GLenum, const void *,
                           GLsizei);
HYBRIS_IMPLEMENT_FUNCTION3(void, glProgramParameteri, GLuint, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glInvalidateFramebuffer, GLenum, GLsizei,
                           const GLenum *);
HYBRIS_IMPLEMENT_FUNCTION7(void, glInvalidateSubFramebuffer, GLenum, GLsizei,
                           const GLenum *, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glTexStorage2D, GLenum, GLsizei, GLenum,
                           GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION6(void, glTexStorage3D, GLenum, GLsizei, GLenum,
                           GLsizei, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glGetInternalformativ, GLenum, GLenum, GLenum,
                           GLsizei, GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glEGLImageTargetTexture2DOES, GLenum,
                           GLeglImageOES);

/* GLES 3.1 */

HYBRIS_IMPLEMENT_FUNCTION3(void, glDispatchCompute, GLuint, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDispatchComputeIndirect, GLintptr);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDrawArraysIndirect, GLenum, const void *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glDrawElementsIndirect, GLenum, GLenum,
                           const void *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glFramebufferParameteri, GLenum, GLenum,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetFramebufferParameteriv, GLenum, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetProgramInterfaceiv, GLuint, GLenum,
                           GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(GLuint, glGetProgramResourceIndex, GLuint, GLenum,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION6(void, glGetProgramResourceName, GLuint, GLenum,
                           GLuint, GLsizei, GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION8(void, glGetProgramResourceiv, GLuint, GLenum, GLuint,
                           GLsizei, const GLenum *, GLsizei, GLsizei *,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(GLint, glGetProgramResourceLocation, GLuint, GLenum,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glUseProgramStages, GLuint, GLbitfield,
                           GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glActiveShaderProgram, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(GLuint, glCreateShaderProgramv, GLenum, GLsizei,
                           const GLchar *const *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glBindProgramPipeline, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteProgramPipelines, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenProgramPipelines, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsProgramPipeline, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetProgramPipelineiv, GLuint, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glProgramUniform1i, GLuint, GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform2i, GLuint, GLint, GLint,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniform3i, GLuint, GLint, GLint,
                           GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION6(void, glProgramUniform4i, GLuint, GLint, GLint,
                           GLint, GLint, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glProgramUniform1ui, GLuint, GLint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform2ui, GLuint, GLint, GLuint,
                           GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniform3ui, GLuint, GLint, GLuint,
                           GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION6(void, glProgramUniform4ui, GLuint, GLint, GLuint,
                           GLuint, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glProgramUniform1f, GLuint, GLint, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform2f, GLuint, GLint, GLfloat,
                           GLfloat);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniform3f, GLuint, GLint, GLfloat,
                           GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION6(void, glProgramUniform4f, GLuint, GLint, GLfloat,
                           GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform1iv, GLuint, GLint, GLsizei,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform2iv, GLuint, GLint, GLsizei,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform3iv, GLuint, GLint, GLsizei,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform4iv, GLuint, GLint, GLsizei,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform1uiv, GLuint, GLint, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform2uiv, GLuint, GLint, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform3uiv, GLuint, GLint, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform4uiv, GLuint, GLint, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform1fv, GLuint, GLint, GLsizei,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform2fv, GLuint, GLint, GLsizei,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform3fv, GLuint, GLint, GLsizei,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glProgramUniform4fv, GLuint, GLint, GLsizei,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix2fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix3fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix4fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix2x3fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix3x2fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix2x4fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix4x2fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix3x4fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glProgramUniformMatrix4x3fv, GLuint, GLint,
                           GLsizei, GLboolean, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glValidateProgramPipeline, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetProgramPipelineInfoLog, GLuint, GLsizei,
                           GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION7(void, glBindImageTexture, GLuint, GLuint, GLint,
                           GLboolean, GLint, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetBooleani_v, GLenum, GLuint, GLboolean *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMemoryBarrier, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMemoryBarrierByRegion, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION6(void, glTexStorage2DMultisample, GLenum, GLsizei,
                           GLenum, GLsizei, GLsizei, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetMultisamplefv, GLenum, GLuint, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glSampleMaski, GLuint, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetTexLevelParameteriv, GLenum, GLint,
                           GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetTexLevelParameterfv, GLenum, GLint,
                           GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBindVertexBuffer, GLuint, GLuint, GLintptr,
                           GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glVertexAttribFormat, GLuint, GLint, GLenum,
                           GLboolean, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glVertexAttribIFormat, GLuint, GLint, GLenum,
                           GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexAttribBinding, GLuint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glVertexBindingDivisor, GLuint, GLuint);

/* GLES 3.2 */

HYBRIS_IMPLEMENT_FUNCTION0(void, glBlendBarrier);
HYBRIS_IMPLEMENT_FUNCTION15(void, glCopyImageSubData, GLuint, GLenum, GLint,
                            GLint, GLint, GLint, GLuint, GLenum, GLint, GLint,
                            GLint, GLint, GLsizei, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION6(void, glDebugMessageControl, GLenum, GLenum, GLenum,
                           GLsizei, const GLuint *, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION6(void, glDebugMessageInsert, GLenum, GLenum, GLuint,
                           GLenum, GLsizei, const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDebugMessageCallback, GLDEBUGPROC,
                           const void *);
HYBRIS_IMPLEMENT_FUNCTION8(GLuint, glGetDebugMessageLog, GLuint, GLsizei,
                           GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *,
                           GLchar *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glPushDebugGroup, GLenum, GLuint, GLsizei,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION0(void, glPopDebugGroup);
HYBRIS_IMPLEMENT_FUNCTION4(void, glObjectLabel, GLenum, GLuint, GLsizei,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glGetObjectLabel, GLenum, GLuint, GLsizei,
                           GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glObjectPtrLabel, const void *, GLsizei,
                           const GLchar *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetObjectPtrLabel, const void *, GLsizei,
                           GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetPointerv, GLenum, void **);
HYBRIS_IMPLEMENT_FUNCTION2(void, glEnablei, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDisablei, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBlendEquationi, GLuint, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glBlendEquationSeparatei, GLuint, GLenum,
                           GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glBlendFunci, GLuint, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION5(void, glBlendFuncSeparatei, GLuint, GLenum, GLenum,
                           GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION5(void, glColorMaski, GLuint, GLboolean, GLboolean,
                           GLboolean, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION2(GLboolean, glIsEnabledi, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glDrawElementsBaseVertex, GLenum, GLsizei,
                           GLenum, const void *, GLint);
HYBRIS_IMPLEMENT_FUNCTION7(void, glDrawRangeElementsBaseVertex, GLenum, GLuint,
                           GLuint, GLsizei, GLenum, const void *, GLint);
HYBRIS_IMPLEMENT_FUNCTION6(void, glDrawElementsInstancedBaseVertex, GLenum,
                           GLsizei, GLenum, const void *, GLsizei, GLint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glFramebufferTexture, GLenum, GLenum, GLuint,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION8(void, glPrimitiveBoundingBox, GLfloat, GLfloat,
                           GLfloat, GLfloat, GLfloat, GLfloat, GLfloat,
                           GLfloat);
HYBRIS_IMPLEMENT_FUNCTION0(GLenum, glGetGraphicsResetStatus);
HYBRIS_IMPLEMENT_FUNCTION8(void, glReadnPixels, GLint, GLint, GLsizei, GLsizei,
                           GLenum, GLenum, GLsizei, void *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetnUniformfv, GLuint, GLint, GLsizei,
                           GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetnUniformiv, GLuint, GLint, GLsizei,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetnUniformuiv, GLuint, GLint, GLsizei,
                           GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMinSampleShading, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPatchParameteri, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterIiv, GLenum, GLenum,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterIuiv, GLenum, GLenum,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameterIiv, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameterIuiv, GLenum, GLenum,
                           GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glSamplerParameterIiv, GLuint, GLenum,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glSamplerParameterIuiv, GLuint, GLenum,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetSamplerParameterIiv, GLuint, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetSamplerParameterIuiv, GLuint, GLenum,
                           GLuint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexBuffer, GLenum, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glTexBufferRange, GLenum, GLenum, GLuint,
                           GLintptr, GLsizeiptr);
HYBRIS_IMPLEMENT_FUNCTION7(void, glTexStorage3DMultisample, GLenum, GLsizei,
                           GLenum, GLsizei, GLsizei, GLsizei, GLboolean);
