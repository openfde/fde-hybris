#define GL_GLEXT_PROTOTYPES
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>

#define HYBRIS_LIBNAME "libGLESv1_CM.so.1"  // soname
#define HYBRIS_ENVNAME "HYBRIS-GLESv1"
#include "binding.h"

HYBRIS_IMPLEMENT_FUNCTION2(void, glAlphaFunc, GLenum, GLclampf);
HYBRIS_IMPLEMENT_FUNCTION4(void, glClearColor, GLclampf, GLclampf, GLclampf,
                           GLclampf);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearDepthf, GLclampf);
HYBRIS_IMPLEMENT_FUNCTION2(void, glClipPlanef, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColor4f, GLfloat, GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDepthRangef, GLclampf, GLclampf);
HYBRIS_IMPLEMENT_FUNCTION2(void, glFogf, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glFogfv, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION6(void, glFrustumf, GLfloat, GLfloat, GLfloat, GLfloat,
                           GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetClipPlanef, GLenum,
                           GLfloat *); /* was: GLfloat[4] */
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetFloatv, GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetLightfv, GLenum, GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetMaterialfv, GLenum, GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexEnvfv, GLenum, GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameterfv, GLenum, GLenum,
                           GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glLightModelf, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glLightModelfv, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glLightf, GLenum, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glLightfv, GLenum, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLineWidth, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLoadMatrixf, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glMaterialf, GLenum, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glMaterialfv, GLenum, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMultMatrixf, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glMultiTexCoord4f, GLenum, GLfloat, GLfloat,
                           GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glNormal3f, GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION6(void, glOrthof, GLfloat, GLfloat, GLfloat, GLfloat,
                           GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPointParameterf, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPointParameterfv, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glPointSize, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPolygonOffset, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION4(void, glRotatef, GLfloat, GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glScalef, GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvf, GLenum, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvfv, GLenum, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterf, GLenum, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterfv, GLenum, GLenum,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTranslatef, GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glActiveTexture, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glAlphaFuncx, GLenum, GLclampx);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindBuffer, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindTexture, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBlendFunc, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBufferData, GLenum, GLsizeiptr,
                           const GLvoid *, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBufferSubData, GLenum, GLintptr, GLsizeiptr,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClear, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION4(void, glClearColorx, GLclampx, GLclampx, GLclampx,
                           GLclampx);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearDepthx, GLclampx);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearStencil, GLint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClientActiveTexture, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glClipPlanex, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColor4ub, GLubyte, GLubyte, GLubyte,
                           GLubyte);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColor4x, GLfixed, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColorMask, GLboolean, GLboolean, GLboolean,
                           GLboolean);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColorPointer, GLint, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION8(void, glCompressedTexImage2D, GLenum, GLint, GLenum,
                           GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION9(void, glCompressedTexSubImage2D, GLenum, GLint,
                           GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION8(void, glCopyTexImage2D, GLenum, GLint, GLenum, GLint,
                           GLint, GLsizei, GLsizei, GLint);
HYBRIS_IMPLEMENT_FUNCTION8(void, glCopyTexSubImage2D, GLenum, GLint, GLint,
                           GLint, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION1(void, glCullFace, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteBuffers, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteTextures, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDepthFunc, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDepthMask, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDepthRangex, GLclampx, GLclampx);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDisable, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDisableClientState, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glDrawArrays, GLenum, GLint, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION4(void, glDrawElements, GLenum, GLsizei, GLenum,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEnable, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEnableClientState, GLenum);
HYBRIS_IMPLEMENT_FUNCTION0(void, glFinish);
HYBRIS_IMPLEMENT_FUNCTION0(void, glFlush);
HYBRIS_IMPLEMENT_FUNCTION2(void, glFogx, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glFogxv, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glFrontFace, GLenum);
HYBRIS_IMPLEMENT_FUNCTION6(void, glFrustumx, GLfixed, GLfixed, GLfixed, GLfixed,
                           GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetBooleanv, GLenum, GLboolean *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetBufferParameteriv, GLenum, GLenum,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetClipPlanex, GLenum,
                           GLfixed *); /* was: GLfixed[4] */
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenBuffers, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenTextures, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION0(GLenum, glGetError);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetFixedv, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetIntegerv, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetLightxv, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetMaterialxv, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetPointerv, GLenum, GLvoid **);
HYBRIS_IMPLEMENT_FUNCTION1(const GLubyte *, glGetString, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexEnviv, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexEnvxv, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameteriv, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameterxv, GLenum, GLenum,
                           GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glHint, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsBuffer, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsEnabled, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsTexture, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glLightModelx, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glLightModelxv, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glLightx, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glLightxv, GLenum, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLineWidthx, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION0(void, glLoadIdentity);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLoadMatrixx, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLogicOp, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glMaterialx, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glMaterialxv, GLenum, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMatrixMode, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMultMatrixx, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glMultiTexCoord4x, GLenum, GLfixed, GLfixed,
                           GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glNormal3x, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glNormalPointer, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION6(void, glOrthox, GLfixed, GLfixed, GLfixed, GLfixed,
                           GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPixelStorei, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPointParameterx, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPointParameterxv, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glPointSizex, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPolygonOffsetx, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION0(void, glPopMatrix);
HYBRIS_IMPLEMENT_FUNCTION0(void, glPushMatrix);
HYBRIS_IMPLEMENT_FUNCTION7(void, glReadPixels, GLint, GLint, GLsizei, GLsizei,
                           GLenum, GLenum, GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glRotatex, GLfixed, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glSampleCoverage, GLclampf, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION2(void, glSampleCoveragex, GLclampx, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION3(void, glScalex, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION4(void, glScissor, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION1(void, glShadeModel, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glStencilFunc, GLenum, GLint, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glStencilMask, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glStencilOp, GLenum, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glTexCoordPointer, GLint, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvi, GLenum, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvx, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnviv, GLenum, GLenum, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvxv, GLenum, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION9(void, glTexImage2D, GLenum, GLint, GLint, GLsizei,
                           GLsizei, GLint, GLenum, GLenum, const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameteri, GLenum, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterx, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameteriv, GLenum, GLenum,
                           const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterxv, GLenum, GLenum,
                           const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION9(void, glTexSubImage2D, GLenum, GLint, GLint, GLint,
                           GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTranslatex, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION4(void, glVertexPointer, GLint, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glViewport, GLint, GLint, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION3(void, glPointSizePointerOES, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBlendEquationSeparateOES, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glBlendFuncSeparateOES, GLenum, GLenum, GLenum,
                           GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(void, glBlendEquationOES, GLenum);
HYBRIS_IMPLEMENT_FUNCTION5(void, glDrawTexsOES, GLshort, GLshort, GLshort,
                           GLshort, GLshort);
HYBRIS_IMPLEMENT_FUNCTION5(void, glDrawTexiOES, GLint, GLint, GLint, GLint,
                           GLint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glDrawTexxOES, GLfixed, GLfixed, GLfixed,
                           GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDrawTexsvOES, const GLshort *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDrawTexivOES, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDrawTexxvOES, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glDrawTexfOES, GLfloat, GLfloat, GLfloat,
                           GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDrawTexfvOES, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glEGLImageTargetRenderbufferStorageOES, GLenum,
                           GLeglImageOES);
HYBRIS_IMPLEMENT_FUNCTION2(void, glAlphaFuncxOES, GLenum, GLclampx);
HYBRIS_IMPLEMENT_FUNCTION4(void, glClearColorxOES, GLclampx, GLclampx, GLclampx,
                           GLclampx);
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearDepthxOES, GLclampx);
HYBRIS_IMPLEMENT_FUNCTION2(void, glClipPlanexOES, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glColor4xOES, GLfixed, GLfixed, GLfixed,
                           GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDepthRangexOES, GLclampx, GLclampx);
HYBRIS_IMPLEMENT_FUNCTION2(void, glFogxOES, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glFogxvOES, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION6(void, glFrustumxOES, GLfixed, GLfixed, GLfixed,
                           GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetClipPlanexOES, GLenum,
                           GLfixed *); /* was: GLfixed[4] */
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetFixedvOES, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetLightxvOES, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetMaterialxvOES, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexEnvxvOES, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexParameterxvOES, GLenum, GLenum,
                           GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glLightModelxOES, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glLightModelxvOES, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glLightxOES, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glLightxvOES, GLenum, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLineWidthxOES, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION1(void, glLoadMatrixxOES, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glMaterialxOES, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glMaterialxvOES, GLenum, GLenum,
                           const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glMultMatrixxOES, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glMultiTexCoord4xOES, GLenum, GLfixed, GLfixed,
                           GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glNormal3xOES, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION6(void, glOrthoxOES, GLfixed, GLfixed, GLfixed,
                           GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPointParameterxOES, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPointParameterxvOES, GLenum,
                           const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glPointSizexOES, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glPolygonOffsetxOES, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION4(void, glRotatexOES, GLfixed, GLfixed, GLfixed,
                           GLfixed);
HYBRIS_IMPLEMENT_FUNCTION2(void, glSampleCoveragexOES, GLclampx, GLboolean);
HYBRIS_IMPLEMENT_FUNCTION3(void, glScalexOES, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvxOES, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexEnvxvOES, GLenum, GLenum,
                           const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterxOES, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexParameterxvOES, GLenum, GLenum,
                           const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTranslatexOES, GLfixed, GLfixed, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsRenderbufferOES, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindRenderbufferOES, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteRenderbuffersOES, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenRenderbuffersOES, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glRenderbufferStorageOES, GLenum, GLenum,
                           GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetRenderbufferParameterivOES, GLenum,
                           GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsFramebufferOES, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glBindFramebufferOES, GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteFramebuffersOES, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenFramebuffersOES, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLenum, glCheckFramebufferStatusOES, GLenum);
HYBRIS_IMPLEMENT_FUNCTION4(void, glFramebufferRenderbufferOES, GLenum, GLenum,
                           GLenum, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glFramebufferTexture2DOES, GLenum, GLenum,
                           GLenum, GLuint, GLint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetFramebufferAttachmentParameterivOES,
                           GLenum, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glGenerateMipmapOES, GLenum);
HYBRIS_IMPLEMENT_FUNCTION2(void *, glMapBufferOES, GLenum, GLenum);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glUnmapBufferOES, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetBufferPointervOES, GLenum, GLenum,
                           GLvoid **);
HYBRIS_IMPLEMENT_FUNCTION1(void, glCurrentPaletteMatrixOES, GLuint);
HYBRIS_IMPLEMENT_FUNCTION0(void, glLoadPaletteFromModelViewMatrixOES);
HYBRIS_IMPLEMENT_FUNCTION4(void, glMatrixIndexPointerOES, GLint, GLenum,
                           GLsizei, const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glWeightPointerOES, GLint, GLenum, GLsizei,
                           const GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION2(GLbitfield, glQueryMatrixxOES, GLfixed *, GLint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDepthRangefOES, GLclampf, GLclampf);
HYBRIS_IMPLEMENT_FUNCTION6(void, glFrustumfOES, GLfloat, GLfloat, GLfloat,
                           GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION6(void, glOrthofOES, GLfloat, GLfloat, GLfloat,
                           GLfloat, GLfloat, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION2(void, glClipPlanefOES, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGetClipPlanefOES, GLenum,
                           GLfloat *); /* was: GLfloat[4] */
HYBRIS_IMPLEMENT_FUNCTION1(void, glClearDepthfOES, GLclampf);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexGenfOES, GLenum, GLenum, GLfloat);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexGenfvOES, GLenum, GLenum,
                           const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexGeniOES, GLenum, GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexGenivOES, GLenum, GLenum, const GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexGenxOES, GLenum, GLenum, GLfixed);
HYBRIS_IMPLEMENT_FUNCTION3(void, glTexGenxvOES, GLenum, GLenum,
                           const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexGenfvOES, GLenum, GLenum, GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexGenivOES, GLenum, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetTexGenxvOES, GLenum, GLenum, GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glBindVertexArrayOES, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteVertexArraysOES, GLsizei,
                           const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenVertexArraysOES, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsVertexArrayOES, GLuint);
HYBRIS_IMPLEMENT_FUNCTION5(void, glRenderbufferStorageMultisampleAPPLE, GLenum,
                           GLsizei, GLenum, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION0(void, glResolveMultisampleFramebufferAPPLE);
HYBRIS_IMPLEMENT_FUNCTION3(void, glDiscardFramebufferEXT, GLenum, GLsizei,
                           const GLenum *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glMultiDrawArraysEXT, GLenum, const GLint *,
                           const GLsizei *, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION5(void, glMultiDrawElementsEXT, GLenum,
                           const GLsizei *, GLenum, const void *const *,
                           GLsizei);
HYBRIS_IMPLEMENT_FUNCTION2(void, glClipPlanefIMG, GLenum, const GLfloat *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glClipPlanexIMG, GLenum, const GLfixed *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glRenderbufferStorageMultisampleIMG, GLenum,
                           GLsizei, GLenum, GLsizei, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION6(void, glFramebufferTexture2DMultisampleIMG, GLenum,
                           GLenum, GLenum, GLuint, GLint, GLsizei);
HYBRIS_IMPLEMENT_FUNCTION2(void, glDeleteFencesNV, GLsizei, const GLuint *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glGenFencesNV, GLsizei, GLuint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glIsFenceNV, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glTestFenceNV, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetFenceivNV, GLuint, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glFinishFenceNV, GLuint);
HYBRIS_IMPLEMENT_FUNCTION2(void, glSetFenceNV, GLuint, GLenum);
HYBRIS_IMPLEMENT_FUNCTION3(void, glGetDriverControlsQCOM, GLint *, GLsizei,
                           GLuint *);
HYBRIS_IMPLEMENT_FUNCTION4(void, glGetDriverControlStringQCOM, GLuint, GLsizei,
                           GLsizei *, GLchar *);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEnableDriverControlQCOM, GLuint);
HYBRIS_IMPLEMENT_FUNCTION1(void, glDisableDriverControlQCOM, GLuint);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtGetTexturesQCOM, GLuint *, GLint,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtGetBuffersQCOM, GLuint *, GLint, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtGetRenderbuffersQCOM, GLuint *, GLint,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtGetFramebuffersQCOM, GLuint *, GLint,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glExtGetTexLevelParameterivQCOM, GLuint,
                           GLenum, GLint, GLenum, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtTexObjectStateOverrideiQCOM, GLenum,
                           GLenum, GLint);
HYBRIS_IMPLEMENT_FUNCTION11(void, glExtGetTexSubImageQCOM, GLenum, GLint, GLint,
                            GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum,
                            GLenum, GLvoid *);
HYBRIS_IMPLEMENT_FUNCTION2(void, glExtGetBufferPointervQCOM, GLenum, GLvoid **);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtGetShadersQCOM, GLuint *, GLint, GLint *);
HYBRIS_IMPLEMENT_FUNCTION3(void, glExtGetProgramsQCOM, GLuint *, GLint,
                           GLint *);
HYBRIS_IMPLEMENT_FUNCTION1(GLboolean, glExtIsProgramBinaryQCOM, GLuint);
HYBRIS_IMPLEMENT_FUNCTION4(void, glExtGetProgramBinarySourceQCOM, GLuint,
                           GLenum, GLchar *, GLint *);
HYBRIS_IMPLEMENT_FUNCTION5(void, glStartTilingQCOM, GLuint, GLuint, GLuint,
                           GLuint, GLbitfield);
HYBRIS_IMPLEMENT_FUNCTION1(void, glEndTilingQCOM, GLbitfield);

HYBRIS_IMPLEMENT_FUNCTION2(void, glEGLImageTargetTexture2DOES, GLenum,
                           GLeglImageOES);
