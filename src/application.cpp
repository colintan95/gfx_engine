#include "application.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include "resource/image_loader.h"
#include "resource/model_loader.h"
#include "window/window.h"

constexpr int kScreenWidth = 1920;
constexpr int kScreenHeight = 1080;

constexpr float kAspectRatio = 
    static_cast<float>(kScreenWidth) / static_cast<float>(kScreenHeight);

const char kVertShaderSrc[] =
    "#version 330 core\n"
    "layout(location = 0) in vec3 vert_pos;\n"
    "layout(location = 2) in vec2 vert_texcoord;\n"
    "out vec2 frag_texcoord;\n"
    "uniform mat4 mvp_mat;\n"
    "void main() {\n"
    "  frag_texcoord = vert_texcoord;\n"
    "  gl_Position = mvp_mat * vec4(vert_pos, 1.0);\n"
    "}";
const char kFragShaderSrc[] =
    "#version 330 core\n"
    "in vec2 frag_texcoord;\n"
    "out vec4 out_color;\n"
    "uniform sampler2D tex_sampler;\n"
    "void main() {\n"
    "  out_color = texture(tex_sampler, frag_texcoord);\n"
    "}";

bool CheckShaderSuccess(int shader) {
  int gl_success = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &gl_success);

  if (gl_success == GL_TRUE) {
    return true;
  } else {
    char log[256];
    glGetShaderInfoLog(shader, 256, nullptr, log);

    std::cerr << "Failed to compile shader." << std::endl;
    std::cerr << log << std::endl;

    return false;
  }
}

bool CheckProgramSuccess(int program) {
  int gl_success = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &gl_success);

  if (gl_success == GL_TRUE) {
    return true;
  } else {
    char log[256];
    glGetShaderInfoLog(program, 256, nullptr, log);

    std::cerr << "Failed to link program." << std::endl;
    std::cerr << log << std::endl;

    return false;
  }
}

Application::Application() {
  try {
    window_ = std::make_unique<window::Window>(kScreenWidth, kScreenHeight, "Hello World");
  } catch (window::WindowInitException& e) {
    // TODO(colintan): Do something else
    std::exit(EXIT_FAILURE);
  } 

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  resource::ModelLoader model_loader;
  model_ = model_loader.LoadModel("assets/cube/cube.obj");
  if (model_ == nullptr) {
    std::cerr << "Failed to load model." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  resource::ImageLoader image_loader;
  std::shared_ptr<resource::Image> image = image_loader.LoadImage("assets/cube/default.png");
  if (image == nullptr) {
    std::cerr << "Failed to load image." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  const char* vert_shader_src = static_cast<const char*>(kVertShaderSrc);
  glShaderSource(vert_shader, 1, &vert_shader_src, nullptr);
  glCompileShader(vert_shader);
  if (!CheckShaderSuccess(vert_shader)) {
    std::exit(EXIT_FAILURE);
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  const char* frag_shader_src = static_cast<const char*>(kFragShaderSrc);
  glShaderSource(frag_shader, 1, &frag_shader_src, nullptr);
  glCompileShader(frag_shader);
  if (!CheckShaderSuccess(frag_shader)) {
    std::exit(EXIT_FAILURE);
  }

  program_ = glCreateProgram();
  glAttachShader(program_, vert_shader);
  glAttachShader(program_, frag_shader);
  glLinkProgram(program_);
  if (!CheckProgramSuccess(program_)) {
    std::exit(EXIT_FAILURE);
  }
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

  glUseProgram(program_);

  glm::mat4 model_mat{1.f};
  glm::mat4 view_mat = 
      glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -10.f}) *
      glm::rotate(glm::mat4{1.f}, glm::radians(-15.f), glm::vec3{0.f, 1.f, 0.f});

  glm::mat4 proj_mat = glm::perspective(glm::radians(30.f), kAspectRatio, 0.1f, 1000.f);
  glm::mat4 mvp_mat = proj_mat * view_mat * model_mat;

  GLint mvp_mat_loc = glGetUniformLocation(program_, "mvp_mat");
  glUniformMatrix4fv(mvp_mat_loc, 1, GL_FALSE, glm::value_ptr(mvp_mat));

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  assert(image->format == resource::ImageFormat::RGB);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE,
               image->pixels.data());

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture_);

  GLint texture_loc = glGetUniformLocation(program_, "tex_sampler");
  glUniform1i(texture_loc, 1);

  glCreateVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glCreateBuffers(1, &pos_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, pos_vbo_);
  glBufferData(GL_ARRAY_BUFFER, model_->positions.size() * sizeof(glm::vec3), 
               glm::value_ptr(model_->positions[0]), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);

  glCreateBuffers(1, &texcoord_vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, texcoord_vbo_);
  glBufferData(GL_ARRAY_BUFFER, model_->texcoords.size() * sizeof(glm::vec3), 
               glm::value_ptr(model_->texcoords[0]), GL_STATIC_DRAW);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glViewport(0, 0, kScreenWidth, kScreenHeight);
}

Application::~Application() {
  glDeleteBuffers(1, &texcoord_vbo_);
  glDeleteBuffers(1, &pos_vbo_);
  glDeleteVertexArrays(1, &vao_);
  glDeleteTextures(1, &texture_);
  glDeleteProgram(program_);
  
  window_.reset();
}

void Application::RunLoop() {
  while (!window_->ShouldClose()) {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, model_->faces * 3);

    glBindVertexArray(0);

    window_->Tick();
  }
}