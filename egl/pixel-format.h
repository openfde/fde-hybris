#pragma once

#include <GLES/gl.h>

#include <cstdint>

namespace egl {

class HalPixelFormat {
 public:
  HalPixelFormat() = default;
  explicit HalPixelFormat(int32_t format);
  bool BuildFormat(int32_t red_size, int32_t green_size, int32_t blue_size,
                   int32_t alpha_size);

  int32_t RedSize() const { return red_size_; }
  int32_t GreenSize() const { return green_size_; }
  int32_t BlueSize() const { return blue_size_; }
  int32_t AlphaSize() const { return alpha_size_; }

  int32_t PixelFormat() const { return format_; }

  GLenum TextureInternalFormat() const { return tex_internal_format_; }

 private:
  int32_t red_size_{};
  int32_t green_size_{};
  int32_t blue_size_{};
  int32_t alpha_size_{};

  GLenum tex_internal_format_{};

  int32_t format_ = -1;
};

}  // namespace egl