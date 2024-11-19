#include "texture-flip.h"

#include <GLES3/gl3.h>
#include <log/log.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

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

    "void main(void) {\n"
    "  gl_Position = vec4(position.x, position.y, position.z, position.w);\n"
    "  outCoord = inCoord;\n"
    "}\n";

// Similarly, just interpolate texture coordinates.
const char kFragmentShaderSource[] =
    "#define kComposeModeDevice 2\n"
    "precision mediump float;\n"
    "varying lowp vec2 outCoord;\n"
    "uniform sampler2D tex;\n"

    "void main(void) {\n"
    "  gl_FragColor = texture2D(tex, outCoord);\n"
    "}\n";

// Hard-coded arrays of vertex information.
struct Vertex {
  float pos[3];
  float coord[2];
};

const Vertex kVertices[] = {
    // flip vertically
    {{-1, -1, +0}, {+0, +1}},
    {{+1, -1, +0}, {+1, +1}},
    {{-1, +1, +0}, {+0, +0}},
    {{+1, +1, +0}, {+1, +0}},
};

class ShadeContextGuard {
 public:
  ShadeContextGuard() {
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &bound_element_array_);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bound_array_);
    glGetIntegerv(GL_CURRENT_PROGRAM, &prev_program_);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prev_active_texture_unit_);
  }

  ~ShadeContextGuard() {
    glUseProgram(static_cast<GLuint>(prev_program_));

    glActiveTexture(prev_active_texture_unit_);
    glBindTexture(GL_TEXTURE_2D, prev_bind_texture_);

    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(bound_array_));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLuint>(bound_element_array_));
  }

 private:
  GLint bound_element_array_ = {};
  GLint bound_array_ = {};
  GLint prev_program_ = {};
  GLint prev_active_texture_unit_ = GL_TEXTURE0;
  GLint prev_bind_texture_ = {};
};

class VertexArrayBound {
 public:
  VertexArrayBound(GLint vao) : vertex_array_buffer_(vao) {
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev_vertex_array_);
    if (prev_vertex_array_ != vertex_array_buffer_) {
      glBindVertexArray(vertex_array_buffer_);
    }
  }

  VertexArrayBound(GLuint vao) : VertexArrayBound(static_cast<GLint>(vao)) {}

  ~VertexArrayBound() {
    if (prev_vertex_array_ != vertex_array_buffer_) {
      glBindVertexArray(prev_vertex_array_);
    }
  }

 private:
  GLint vertex_array_buffer_ = {};
  GLint prev_vertex_array_ = {};
};

}  // namespace

TextureFlipPtr TextureFlip::Create() {
  TextureFlipPtr flip;
  if (auto version_str = reinterpret_cast<const char*>(glGetString(GL_VERSION));
      version_str) {
    if (strstr(version_str, "OpenGL ES 3.")) {
      flip = std::make_shared<TextureFlipGles3>();
    } else if (strstr(version_str, "OpenGL ES 2.")) {
      flip = std::make_shared<TextureFlipGles2>();
    } else {
      ALOGE("Unsupported OpenGL ES version");
    }
  }
  if (flip) {
    flip->Initialize();
  }
  return flip;
}

TextureFlipPtr TextureFlip::Instance() {
  static TextureFlipPtr flip{};
  static std::mutex mtx{};
  static bool inited = false;
  if (!inited) {
    std::lock_guard<std::mutex> guard{mtx};
    if (!inited) {
      flip = Create();
      std::atomic_thread_fence(std::memory_order::memory_order_release);
      inited = true;
    }
  }

  if (!flip->InCurrentContext()) {
    return Create();
  }
  return flip;
}

bool TextureFlip::Initialize() {
  if (program_) {
    return true;
  }
  // Create shaders and program.
  vertex_shader_ = createShader(GL_VERTEX_SHADER, kVertexShaderSource);
  fragment_shader_ = createShader(GL_FRAGMENT_SHADER, kFragmentShaderSource);

  program_ = glCreateProgram();
  glAttachShader(program_, vertex_shader_);
  glAttachShader(program_, fragment_shader_);

  GLint success;
  glLinkProgram(program_);
  glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (success == GL_FALSE) {
    GLchar messages[256];
    glGetProgramInfoLog(program_, sizeof(messages), 0, &messages[0]);
    ALOGE("%s: Could not create/link program: %s", __FUNCTION__, messages);
    glDeleteProgram(program_);
    program_ = 0;
    return false;
  }

  ShadeContextGuard guard;

  glUseProgram(program_);

  // Retrieve attribute/uniform locations.
  position_slot_ = glGetAttribLocation(program_, "position");
  in_coord_slot_ = glGetAttribLocation(program_, "inCoord");
  texture_slot_ = glGetUniformLocation(program_, "tex");
  glUniform1i(texture_slot_, 0);

  if (InitLocations()) {
    if (auto err = glGetError(); err != GL_NO_ERROR) {
      ALOGE("%s:%p, program %d initialize error=0x%04X", __FUNCTION__, this,
            program_, err);
    } else {
      // Validate program, just to be sure.
      glValidateProgram(program_);
      GLint validState = 0;
      glGetProgramiv(program_, GL_VALIDATE_STATUS, &validState);
      if (validState == GL_TRUE) {
        return true;
      }
      GLchar messages[256] = {};
      glGetProgramInfoLog(program_, sizeof(messages), 0, &messages[0]);
      ALOGE("%s:%p, Could not run program: '%s'", __FUNCTION__, this, messages);
      glDeleteProgram(program_);
    }
  }
  program_ = 0;
  return false;
}

bool TextureFlip::Flip(GLuint texture) {
  if (!program_) {
    ALOGE("%s:%p no program", __FUNCTION__, this);
    return false;
  }

  // clear error
  glGetError();

  ShadeContextGuard guard;

  glUseProgram(program_);

  // setup the |texture| uniform value.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  if (auto err = glGetError(); err != GL_NO_ERROR) {
    ALOGE("%s:%p Could not use program %d error=0x%04X", __FUNCTION__, this,
          program_, err);
    return false;
  }

  FlipDraw();

  if (auto err = glGetError(); err != GL_NO_ERROR) {
    ALOGE("%s:%p, program %d draw error=0x%04X", __FUNCTION__, this, program_,
          err);
    return false;
  }
  return true;
}

bool TextureFlip::InCurrentContext() const {
  if (!program_) {
    return true;
  }
  return glIsProgram(program_) && glIsBuffer(vertex_buffer_) && InContext();
}

bool TextureFlip::InContext() const { return true; }

TextureFlip::~TextureFlip() {
  if (vertex_buffer_) {
    glDeleteBuffers(1, &vertex_buffer_);
  }
  if (fragment_shader_) {
    glDeleteShader(fragment_shader_);
  }
  if (vertex_shader_) {
    glDeleteShader(vertex_shader_);
  }
  if (program_) {
    glDeleteProgram(program_);
  }
}

bool TextureFlipGles2::InitLocations() {
  // Create vertex and index buffers.
  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);
  return true;
}

void TextureFlipGles2::FlipDraw() {
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  // Setup the |inCoord| attribute values.
  glVertexAttribPointer(position_slot_, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        0);
  glVertexAttribPointer(
      in_coord_slot_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(sizeof(float) * 3)));

  glEnableVertexAttribArray(in_coord_slot_);
  glEnableVertexAttribArray(position_slot_);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  // Disable attributes
  glDisableVertexAttribArray(position_slot_);
  glDisableVertexAttribArray(in_coord_slot_);
}

TextureFlipGles3::~TextureFlipGles3() {
  if (vertex_array_buffer_) {
    glDeleteVertexArrays(1, &vertex_array_buffer_);
  }
}

bool TextureFlipGles3::InitLocations() {
  glGenVertexArrays(1, &vertex_array_buffer_);
  ALOGD("%s:%p, glGenVertexArrays(%d)", __FUNCTION__, this,
        vertex_array_buffer_);
  VertexArrayBound bind{vertex_array_buffer_};
  // Create vertex and index buffers.
  glGenBuffers(1, &vertex_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);

  glVertexAttribPointer(position_slot_, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        0);
  glVertexAttribPointer(
      in_coord_slot_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<GLvoid*>(static_cast<uintptr_t>(sizeof(float) * 3)));

  glEnableVertexAttribArray(in_coord_slot_);
  glEnableVertexAttribArray(position_slot_);

  return true;
}

void TextureFlipGles3::FlipDraw() {
  VertexArrayBound bind{vertex_array_buffer_};
  if (auto err = glGetError(); err != GL_NO_ERROR) {
    ALOGE("%s:%p, bind vertex array buffer %d error=0x%04X", __FUNCTION__, this,
          vertex_array_buffer_, err);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  if (auto err = glGetError(); err != GL_NO_ERROR) {
    ALOGE("%s: draw error=0x%04X", __FUNCTION__, err);
  }
}

bool TextureFlipGles3::InContext() const {
  return glIsVertexArray(vertex_array_buffer_);
}
