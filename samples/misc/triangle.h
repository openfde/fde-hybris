#pragma once

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <string_view>

#include "shader_m.h"

class Triangle {
 public:
  Triangle();
  explicit Triangle(GLsizei src_width, GLsizei src_height);
  void BuildCompileLinkShaders();
  void SetupResources();
  void RenderData();
  void TerminalProgram();

 private:
  void SetupAndConfigrueVertex();

 private:
  static constexpr GLsizei kDefaultSrcWidth = 800;
  static constexpr GLsizei kDefaultSrcHeight = 600;

  static constexpr int kElementSize = 6;
  static const float kVerticeData[];
  GLsizei src_width_ = 0;
  GLsizei src_height_ = 0;

  GLuint vbo_;
  GLuint vao_;

  std::shared_ptr<Shader> shader_;
};
