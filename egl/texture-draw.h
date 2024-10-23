#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

#include <memory>
#include <vector>

// Helper class used to draw a simple texture to the current framebuffer.
// Usage is pretty simple:
//
//   1) Create a TextureDraw instance.
//
//   2) Each time you want to draw a texture, call draw(texture, rotation),
//      where |texture| is the name of a GLES 2.x texture object, and
//      |rotation| is an angle in degrees describing the clockwise rotation
//      in the GL y-upwards coordinate space. This function fills the whole
//      framebuffer with texture content.
//
class TextureDraw {
 public:
  TextureDraw();
  ~TextureDraw();

  bool DrawAndFlip(GLuint texture) { return draw(texture, 0., 0, 0); }

  // Fill the current framebuffer with the content of |texture|, which must
  // be the name of a GLES 2.x texture object. |rotationDegrees| is a
  // clockwise rotation angle in degrees (clockwise in the GL Y-upwards
  // coordinate space; only supported values are 0, 90, 180, 270). |dx,dy| is
  // the translation of the image towards the origin.
  bool draw(GLuint texture, float rotationDegrees, float dx, float dy,
            float clipWidthRatio = 0, float clipHeightRatio = 0) {
    return drawImpl(texture, rotationDegrees, dx, dy, false, clipWidthRatio,
                    clipHeightRatio);
  }

 private:
  bool drawImpl(GLuint texture, float rotationDegrees, float dx, float dy,
                bool wantOverlay, float clipWidthRatio = 0,
                float clipHeightRatio = 0);

  GLuint mVertexShader;
  GLuint mFragmentShader;
  GLuint mProgram;
  GLint mAlpha;
  GLint mComposeMode;
  GLint mColor;
  GLint mCoordTranslation;
  GLint mCoordScale;
  GLint mPositionSlot;
  GLint mInCoordSlot;
  GLint mScaleSlot;
  GLint mTextureSlot;
  GLint mTranslationSlot;
  GLuint mVertexBuffer;
  GLuint mVertexBufferClip;
  float* mClipVertexData;
  float mClipWidthRatio;
  float mClipHeightRatio;
  GLuint mIndexBuffer;
};

using TextureDrawPtr = std::shared_ptr<TextureDraw>;
