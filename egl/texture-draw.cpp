#include "texture-draw.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include <string>

#define ERR(...) fprintf(stderr, __VA_ARGS__)

namespace {

// Helper function to create a new shader.
// |shaderType| is the shader type (e.g. GL_VERTEX_SHADER).
// |shaderText| is a 0-terminated C string for the shader source to use.
// On success, return the handle of the new compiled shader, or 0 on failure.
GLuint createShader(GLint shaderType, const char* shaderText) {
  // Create new shader handle and attach source.
  GLuint shader = glCreateShader(shaderType);
  if (!shader) {
    return 0;
  }
  const GLchar* text = static_cast<const GLchar*>(shaderText);
  const GLint textLen = ::strlen(shaderText);
  glShaderSource(shader, 1, &text, &textLen);

  // Compiler the shader.
  GLint success;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::string infoLog(infoLogLength + 1, '\0');
    fprintf(stderr, "%s: TextureDraw shader compile failed.\n", __func__);
    glGetShaderInfoLog(shader, infoLogLength, 0, &infoLog[0]);
    fprintf(stderr, "%s: Info log:\n%s\n", __func__, infoLog.c_str());
    fprintf(stderr, "%s: Source:\n%s\n", __func__, shaderText);
    glDeleteShader(shader);
  }

  return shader;
}

// No scaling / projection since we want to fill the whole viewport with
// the texture, hence a trivial vertex shader that only supports translation.
// Note: we used to have a proper free-angle rotation support in this shader,
//  but looks like SwiftShader doesn't support either complicated calculations
//  for gl_Position/varyings or just doesn't like trigonometric functions in
//  shader; anyway the new code has hardcoded texture coordinate mapping for
//  different rotation angles and works in both native OpenGL and SwiftShader.
const char kVertexShaderSource[] =
    "attribute vec4 position;\n"
    "attribute vec2 inCoord;\n"
    "varying vec2 outCoord;\n"
    "uniform vec2 translation;\n"
    "uniform vec2 scale;\n"
    "uniform vec2 coordTranslation;\n"
    "uniform vec2 coordScale;\n"

    "void main(void) {\n"
    "  gl_Position.xy = position.xy * scale.xy - translation.xy;\n"
    "  gl_Position.zw = position.zw;\n"
    "  outCoord = inCoord * coordScale + coordTranslation;\n"
    "}\n";

// Similarly, just interpolate texture coordinates.
const char kFragmentShaderSource[] =
    "#define kComposeModeDevice 2\n"
    "precision mediump float;\n"
    "varying lowp vec2 outCoord;\n"
    "uniform sampler2D tex;\n"
    "uniform float alpha;\n"
    "uniform int composeMode;\n"
    "uniform vec4 color ;\n"

    "void main(void) {\n"
    "  if (composeMode == kComposeModeDevice) {\n"
    "    gl_FragColor = alpha * texture2D(tex, outCoord);\n"
    "  } else {\n"
    "    gl_FragColor = alpha * color;\n"
    "  }\n"
    "}\n";

// Hard-coded arrays of vertex information.
struct Vertex {
  float pos[3];
  float coord[2];
};

const Vertex kVertices[] = {
    // 0 degree
    {{+1, -1, +0}, {+1, +0}},
    {{+1, +1, +0}, {+1, +1}},
    {{-1, +1, +0}, {+0, +1}},
    {{-1, -1, +0}, {+0, +0}},
    // 90 degree clock-wise
    {{+1, -1, +0}, {+1, +1}},
    {{+1, +1, +0}, {+0, +1}},
    {{-1, +1, +0}, {+0, +0}},
    {{-1, -1, +0}, {+1, +0}},
    // 180 degree clock-wise
    {{+1, -1, +0}, {+0, +1}},
    {{+1, +1, +0}, {+0, +0}},
    {{-1, +1, +0}, {+1, +0}},
    {{-1, -1, +0}, {+1, +1}},
    // 270 degree clock-wise
    {{+1, -1, +0}, {+0, +0}},
    {{+1, +1, +0}, {+1, +0}},
    {{-1, +1, +0}, {+1, +1}},
    {{-1, -1, +0}, {+0, +1}},
    // flip horizontally
    {{+1, -1, +0}, {+0, +0}},
    {{+1, +1, +0}, {+0, +1}},
    {{-1, +1, +0}, {+1, +1}},
    {{-1, -1, +0}, {+1, +0}},
    // flip vertically
    {{+1, -1, +0}, {+1, +1}},
    {{+1, +1, +0}, {+1, +0}},
    {{-1, +1, +0}, {+0, +0}},
    {{-1, -1, +0}, {+0, +1}},
    // flip source image horizontally, the rotate 90 degrees clock-wise
    {{+1, -1, +0}, {+0, +1}},
    {{+1, +1, +0}, {+1, +1}},
    {{-1, +1, +0}, {+1, +0}},
    {{-1, -1, +0}, {+0, +0}},
    // flip source image vertically, the rotate 90 degrees clock-wise
    {{+1, -1, +0}, {+1, +0}},
    {{+1, +1, +0}, {+0, +0}},
    {{-1, +1, +0}, {+0, +1}},
    {{-1, -1, +0}, {+1, +1}},
};

// Vertex indices for predefined rotation angles.
const GLubyte kIndices[] = {
    0,  1,  2,  2,  3,  0,   // 0
    4,  5,  6,  6,  7,  4,   // 90
    8,  9,  10, 10, 11, 8,   // 180
    12, 13, 14, 14, 15, 12,  // 270
    16, 17, 18, 18, 19, 16,  // flip h
    20, 21, 22, 22, 23, 20,  // flip v
    24, 25, 26, 26, 27, 24,  // flip h, 90
    28, 29, 30, 30, 31, 28   // flip v, 90
};

const GLint kIndicesPerDraw = 6;

class ShadeContextGuard {
 public:
  ShadeContextGuard() {
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &bound_element_array_);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bound_array_);
    glGetIntegerv(GL_CURRENT_PROGRAM, &prev_program_);
  }

  ~ShadeContextGuard() {
    glUseProgram(static_cast<GLuint>(prev_program_));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(bound_array_));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLuint>(bound_element_array_));
  }

 private:
  GLint bound_element_array_ = {};
  GLint bound_array_ = {};
  GLint prev_program_ = {};
};

}  // namespace

TextureDraw::TextureDraw()
    : mVertexShader(0),
      mFragmentShader(0),
      mProgram(0),
      mCoordTranslation(-1),
      mCoordScale(-1),
      mPositionSlot(-1),
      mInCoordSlot(-1),
      mScaleSlot(-1),
      mTextureSlot(-1),
      mTranslationSlot(-1),
      mClipVertexData(nullptr),
      mClipWidthRatio(0),
      mClipHeightRatio(0) {
  // Create shaders and program.
  mVertexShader = createShader(GL_VERTEX_SHADER, kVertexShaderSource);
  mFragmentShader = createShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);

  mProgram = glCreateProgram();
  glAttachShader(mProgram, mVertexShader);
  glAttachShader(mProgram, mFragmentShader);

  GLint success;
  glLinkProgram(mProgram);
  glGetProgramiv(mProgram, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar messages[256];
    glGetProgramInfoLog(mProgram, sizeof(messages), 0, &messages[0]);
    ERR("%s: Could not create/link program: %s\n", __FUNCTION__, messages);
    glDeleteProgram(mProgram);
    mProgram = 0;
    return;
  }

  ShadeContextGuard guard;

  glUseProgram(mProgram);

  // Retrieve attribute/uniform locations.
  mPositionSlot = glGetAttribLocation(mProgram, "position");
  glEnableVertexAttribArray(mPositionSlot);

  mInCoordSlot = glGetAttribLocation(mProgram, "inCoord");
  glEnableVertexAttribArray(mInCoordSlot);

  mAlpha = glGetUniformLocation(mProgram, "alpha");
  mComposeMode = glGetUniformLocation(mProgram, "composeMode");
  mColor = glGetUniformLocation(mProgram, "color");
  mCoordTranslation = glGetUniformLocation(mProgram, "coordTranslation");
  mCoordScale = glGetUniformLocation(mProgram, "coordScale");
  mScaleSlot = glGetUniformLocation(mProgram, "scale");
  mTranslationSlot = glGetUniformLocation(mProgram, "translation");
  mTextureSlot = glGetUniformLocation(mProgram, "tex");

  // set default uniform values
  glUniform1f(mAlpha, 1.0);
  glUniform1i(mComposeMode, 2);
  glUniform2f(mTranslationSlot, 0.0, 0.0);
  glUniform2f(mScaleSlot, 1.0, 1.0);
  glUniform2f(mCoordTranslation, 0.0, 0.0);
  glUniform2f(mCoordScale, 1.0, 1.0);

  // Create vertex and index buffers.
  glGenBuffers(1, &mVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);

  glGenBuffers(1, &mVertexBufferClip);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferClip);
  mClipVertexData = (float*)malloc(sizeof(kVertices));
  memcpy(mClipVertexData, kVertices, sizeof(kVertices));
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), mClipVertexData,
               GL_STREAM_DRAW);

  glGenBuffers(1, &mIndexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices,
               GL_STATIC_DRAW);

  // Reset state.
  glUseProgram(0);
  glDisableVertexAttribArray(mPositionSlot);
  glDisableVertexAttribArray(mInCoordSlot);
}

bool TextureDraw::drawImpl(GLuint texture, float rotation, float dx, float dy,
                           bool wantOverlay, float clipWidthRatio,
                           float clipHeightRatio) {
  if (!mProgram) {
    ERR("%s: no program\n", __FUNCTION__);
    return false;
  }
  ShadeContextGuard guard;

  glUseProgram(mProgram);

#ifndef NDEBUG
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    ERR("%s: Could not use program error=0x%x\n", __FUNCTION__, err);
  }
#endif

  // Setup the |position| attribute values.
  if ((clipWidthRatio > 0) && (clipHeightRatio > 0)) {
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferClip);
    if ((clipWidthRatio != mClipWidthRatio) ||
        (clipHeightRatio != mClipHeightRatio)) {  // need update vertice buffer
      // syslog(LOG_DEBUG, "update vertice buffer, clipWidthRatio = %f,
      // clipHeightRatio = %f", clipWidthRatio, clipHeightRatio);
      for (size_t i = 0; i < sizeof(kVertices) / (sizeof(float));
           i += sizeof(*kVertices) / (sizeof(float))) {
        if (mClipVertexData[i + 3]) mClipVertexData[i + 3] = clipWidthRatio;
        if (mClipVertexData[i + 4]) mClipVertexData[i + 4] = clipHeightRatio;
      }
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(kVertices), mClipVertexData);
      mClipWidthRatio = clipWidthRatio;
      mClipHeightRatio = clipHeightRatio;
    }
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  }

#ifndef NDEBUG
  err = glGetError();
  if (err != GL_NO_ERROR) {
    ERR("%s: Could not bind GL_ARRAY_BUFFER error=0x%x\n", __FUNCTION__, err);
  }
#endif

  glEnableVertexAttribArray(mPositionSlot);
  glVertexAttribPointer(mPositionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        0);

#ifndef NDEBUG
  err = glGetError();
  if (err != GL_NO_ERROR) {
    ERR("%s: Could glVertexAttribPointer with mPositionSlot error=0x%x\n",
        __FUNCTION__, err);
  }
#endif

  // Setup the |inCoord| attribute values.
  glEnableVertexAttribArray(mInCoordSlot);
  glVertexAttribPointer(
      mInCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(sizeof(float) * 3)));

  // setup the |texture| uniform value.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(mTextureSlot, 0);

  // setup the |translation| uniform value.
  glUniform2f(mTranslationSlot, dx, dy);

#ifndef NDEBUG
  // Validate program, just to be sure.
  glValidateProgram(mProgram);
  GLint validState = 0;
  glGetProgramiv(mProgram, GL_VALIDATE_STATUS, &validState);
  if (validState == GL_FALSE) {
    GLchar messages[256] = {};
    glGetProgramInfoLog(mProgram, sizeof(messages), 0, &messages[0]);
    ERR("%s: Could not run program: '%s'\n", __FUNCTION__, messages);
    return false;
  }
#endif

  // Do the rendering.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
#ifndef NDEBUG
  err = glGetError();
  if (err != GL_NO_ERROR) {
    ERR("%s: Could not glBindBuffer(GL_ELEMENT_ARRAY_BUFFER) error=0x%x\n",
        __FUNCTION__, err);
  }
#endif

  // We may only get 0, 90, 180, 270 in |rotation| so far.
  const int intRotation = ((int)rotation) / 90;
  assert(intRotation >= 0 && intRotation <= 3);
  intptr_t indexShift = 0;
  switch (intRotation) {
    case 0:
      indexShift = 5 * kIndicesPerDraw;
      break;
    case 1:
      indexShift = 7 * kIndicesPerDraw;
      break;
    case 2:
      indexShift = 4 * kIndicesPerDraw;
      break;
    case 3:
      indexShift = 6 * kIndicesPerDraw;
      break;
  }

  glDrawElements(GL_TRIANGLES, kIndicesPerDraw, GL_UNSIGNED_BYTE,
                 (const GLvoid*)indexShift);

  GLfloat scale[2];
  glGetUniformfv(mProgram, mScaleSlot, scale);

  if (auto err = glGetError(); err != GL_NO_ERROR) {
    ERR("%s: Could not glDrawElements() error=0x%x\n", __FUNCTION__, err);
  }

  // TODO(digit): Restore previous program state.
  // For now, reset back to zero and assume other users will
  // follow the same protocol.
  glUseProgram(0);
  glDisableVertexAttribArray(mPositionSlot);
  glDisableVertexAttribArray(mInCoordSlot);

  return true;
}

TextureDraw::~TextureDraw() {
  glDeleteBuffers(1, &mIndexBuffer);
  glDeleteBuffers(1, &mVertexBuffer);
  glDeleteBuffers(1, &mVertexBufferClip);
  free(mClipVertexData);

  if (mFragmentShader) {
    glDeleteShader(mFragmentShader);
  }
  if (mVertexShader) {
    glDeleteShader(mVertexShader);
  }
}
