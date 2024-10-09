#include "triangle.h"

#include <chrono>
#include <filesystem>

static const char *kVertexShaderSource =
    "#version 300 es\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out mediump vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\n";

static const char *kFragmentShaderSource =
    "#version 300 es\n"
    "out mediump vec4 FragColor;\n"
    "in mediump vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
    "}\n";

Triangle::Triangle() : Triangle(kDefaultSrcWidth, kDefaultSrcHeight) {}

Triangle::Triangle(GLsizei src_width, GLsizei src_height)
    : src_width_(src_width), src_height_(src_height) {}

void Triangle::BuildCompileLinkShaders() {
  // configure global opengl state
  // -----------------------------
  // glEnable(GL_DEPTH_TEST);

  // build and compile our shader zprogram
  // ------------------------------------
  shader_ =
      std::make_shared<Shader>(kVertexShaderSource, kFragmentShaderSource);
}

void Triangle::SetupResources() {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  this->SetupAndConfigrueVertex();

  // tell opengl for each sampler to which texture unit it belongs to (only has
  // to be done once)
  // -------------------------------------------------------------------------------------------
  shader_->use();
}

const float Triangle::kVerticeData[] = {
    // positions         // colors
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom left
    0.0f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f   // top

};

void Triangle::RenderData() {  // world space positions of our cubes

  glViewport(0, 0, src_width_, src_height_);

  // render
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // activate shader
  shader_->use();
  // render the triangle
  // glUseProgram(shaderProgram);
  if (auto status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
      status != GL_FRAMEBUFFER_COMPLETE) {
  }

  // render boxes
  glBindVertexArray(vao_);

  glDrawArrays(GL_TRIANGLES, 0, 3);

  glFlush();
}

void Triangle::TerminalProgram() {
  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  // glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteProgram(shader_->ID);
}

void Triangle::SetupAndConfigrueVertex() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kVerticeData), kVerticeData,
               GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kElementSize * sizeof(float),
                        (void *)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, kElementSize * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
}
