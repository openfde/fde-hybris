#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include <memory>
#include <vector>

class TextureFlip;
using TextureFlipPtr = std::shared_ptr<TextureFlip>;

class TextureFlip {
 public:
  virtual ~TextureFlip();

  bool Flip(GLuint texture);

  bool Initialize();

  static TextureFlipPtr &Instance();

 protected:
  TextureFlip() = default;
  virtual bool InitLocations() = 0;
  virtual void FlipDraw() = 0;

 protected:
  GLint position_slot_ = {};
  GLint in_coord_slot_ = {};
  GLint texture_slot_ = {};

  GLuint vertex_buffer_ = {};

 private:
  GLuint vertex_shader_ = {};
  GLuint fragment_shader_ = {};
  GLuint program_ = {};
};

class TextureFlipGles2 : public TextureFlip {
 public:
  TextureFlipGles2() = default;
  ~TextureFlipGles2() = default;

 private:
  bool InitLocations() override;
  void FlipDraw() override;
};

class TextureFlipGles3 : public TextureFlip {
 public:
  TextureFlipGles3() = default;
  ~TextureFlipGles3();

 private:
  bool InitLocations() override;
  void FlipDraw() override;

  GLuint vertex_array_buffer_ = {};
};