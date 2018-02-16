#include "renderer.h"
#include "consts.h"
#include "game.h"
#include <glm/gtc/matrix_transform.hpp>
#include "texture.h"

// include stb_image
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include "stb_image.h"

void setFramebufferSize(GLFWwindow* w, int width, int height)
{
  Game& game = *static_cast<Game*>(glfwGetWindowUserPointer(w));
  Renderer& renderer = game.getRenderer();

  renderer.width_ = width;
  renderer.height_ = height;

  renderer.bloomFb_ = {};
  renderer.bloomDepth_ = {};
  renderer.preBloomTex_ = {};
  renderer.bloomPassTex1_ = {};
  renderer.bloomPassTex2_ = {};

  renderer.initializeFramebuffers();
}

bool Renderer::singletonInitialized_ = false;

Renderer::Window::Window()
{
  // Initialize GLFW
  if (!glfwInit())
  {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want version 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Mac requires this
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a window
  window_ = glfwCreateWindow(1024, 768, "Aromatherapy", nullptr, nullptr);
  if (window_ == nullptr)
  {
    throw std::runtime_error("Failed to open GLFW window");
  }

  glfwMakeContextCurrent(window_);

  glewExperimental = true; // Needed in core profile
  if (glewInit() != GLEW_OK)
  {
    throw std::runtime_error("Failed to initialize GLEW");
  }

  glfwSetFramebufferSizeCallback(window_, &setFramebufferSize);
}

Renderer::Window::~Window()
{
  glfwTerminate();
}

Renderer::Renderer() :
  monitor_("res/monitor-old.obj"),
  ntscShader_("ntsc"),
  finalShader_("final"),
  blitShader_("blit"),
  fillShader_("fill"),
  bloom1Shader_("bloom1"),
  bloom2Shader_("bloom2")
{
  if (singletonInitialized_)
  {
    throw std::logic_error("Singleton renderer already initialized");
  }

  singletonInitialized_ = true;

  // Set up vertex array object
  glBindVertexArray(vao_.getId());

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set up the rendering buffers and textures
  glfwGetFramebufferSize(window_.getHandle(), &width_, &height_);

  initializeFramebuffers();

  // Load the vertices of a flat surface
  GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

  glBindBuffer(GL_ARRAY_BUFFER, quadBuffer_.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(GLfloat) * 18,
    g_quad_vertex_buffer_data,
    GL_STATIC_DRAW);

  // Load NTSC artifacts
  int atdw, atdh;
  unsigned char* artifactsData =
    stbi_load("res/artifacts.bmp", &atdw, &atdh, 0, 3);

  flipImageData(artifactsData, atdw, atdh, 3);

  glBindTexture(GL_TEXTURE_2D, artifactsTex_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    atdw,
    atdh,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    artifactsData);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_MIN_FILTER,
    GL_NEAREST_MIPMAP_NEAREST);

  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(artifactsData);

  // Load NTSC scanlines
  unsigned char* scanlinesData =
    stbi_load("res/scanlines_333.bmp", &atdw, &atdh, 0, 3);

  flipImageData(scanlinesData, atdw, atdh, 3);

  glBindTexture(GL_TEXTURE_2D, scanlinesTex_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    atdw,
    atdh,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    scanlinesData);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_MIN_FILTER,
    GL_NEAREST_MIPMAP_NEAREST);

  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(scanlinesData);
}

void Renderer::initializeFramebuffers()
{
  // Set up the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, genericFb_.getId());
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, DrawBuffers);

  // Set up the bloom framebuffer and depthbuffer
  glBindFramebuffer(GL_FRAMEBUFFER, bloomFb_.getId());
  GLenum DrawBuffers2[1] = {GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, DrawBuffers2);

  glBindRenderbuffer(GL_RENDERBUFFER, bloomDepth_.getId());
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER,
    GL_DEPTH_ATTACHMENT,
    GL_RENDERBUFFER,
    bloomDepth_.getId());

  // Set up the NTSC rendering buffers
  glBindTexture(GL_TEXTURE_2D, renderPages_[0].getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    GAME_WIDTH,
    GAME_HEIGHT,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, renderPages_[1].getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    GAME_WIDTH,
    GAME_HEIGHT,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Set up bloom rendering buffers
  glBindTexture(GL_TEXTURE_2D, preBloomTex_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    width_,
    height_,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, bloomPassTex1_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    width_ / 4,
    height_ / 4,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, bloomPassTex2_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    width_ / 4,
    height_ / 4,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Renderer::~Renderer()
{
  singletonInitialized_ = false;
}

void Renderer::fill(Texture& tex, Rectangle dstrect, int r, int g, int b)
{
  // Target the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, genericFb_.getId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.getId(), 0);

  // Set up the vertex attributes
  int width = tex.getWidth();
  int height = tex.getHeight();

  GLfloat minx = (GLfloat) dstrect.x / width * 2.0 - 1.0;
  GLfloat miny = -((GLfloat) dstrect.y / height * 2.0 - 1.0);
  GLfloat maxx = (GLfloat) (dstrect.x + dstrect.w) / width * 2.0 - 1.0;
  GLfloat maxy = -((GLfloat) (dstrect.y + dstrect.h) / height * 2.0 - 1.0);

  GLfloat vertexData[] = {
    minx, miny,
    maxx, miny,
    maxx, maxy,
    minx, miny,
    minx, maxy,
    maxx, maxy
  };

  GLBuffer vertexBuffer;
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(GLfloat) * 12,
    vertexData, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glViewport(0, 0, tex.getWidth(), tex.getHeight());
  glClear(GL_DEPTH_BUFFER_BIT);

  fillShader_.use();
  glUniform3f(
    fillShader_.getUniformLocation("vecColor"),
    r / 255.0,
    g / 255.0,
    b / 255.0);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDisableVertexAttribArray(0);
}

void Renderer::blit(
  const Texture& src,
  Texture& dst,
  Rectangle srcrect,
  Rectangle dstrect,
  double alpha)
{
  alpha = glm::clamp(alpha, 0.0, 1.0);

  // Target the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, genericFb_.getId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dst.getId(), 0);

  // Set up the vertex attributes
  int width = dst.getWidth();
  int height = dst.getHeight();

  GLfloat minx = (GLfloat) dstrect.x / width * 2.0 - 1.0;
  GLfloat miny = -((GLfloat) dstrect.y / height * 2.0 - 1.0);
  GLfloat maxx = (GLfloat) (dstrect.x + dstrect.w) / width * 2.0 - 1.0;
  GLfloat maxy = -((GLfloat) (dstrect.y + dstrect.h) / height * 2.0 - 1.0);

  GLfloat vertexData[] = {
    minx, miny,
    maxx, miny,
    minx, maxy,
    maxx, maxy
  };

  GLBuffer vertexBuffer;
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(GLfloat) * 8,
    vertexData,
    GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  GLfloat minu = (GLfloat) srcrect.x / src.getWidth();
  GLfloat minv = 1 - ((GLfloat) srcrect.y / src.getHeight());
  GLfloat maxu = (GLfloat) (srcrect.x + srcrect.w) / src.getWidth();
  GLfloat maxv = 1 - ((GLfloat) (srcrect.y + srcrect.h) / src.getHeight());

  GLfloat uvData[] = {
    minu, minv,
    maxu, minv,
    minu, maxv,
    maxu, maxv
  };

  GLBuffer uvBuffer;
  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer.getId());
  glBufferData(
    GL_ARRAY_BUFFER,
    sizeof(GLfloat) * 8,
    uvData,
    GL_STATIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Set up the shader
  blitShader_.use();
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, dst.getWidth(), dst.getHeight());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, src.getId());
  glUniform1i(blitShader_.getUniformLocation("srctex"), 0);
  glUniform1f(blitShader_.getUniformLocation("alpha"), alpha);

  // Blit!
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Unload everything
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);
}

void Renderer::bloomPass1(
  const GLTexture& src,
  GLTexture& dst,
  bool horizontal,
  glm::vec2 srcRes,
  glm::vec2 dstRes)
{
  glBindFramebuffer(GL_FRAMEBUFFER, genericFb_.getId());
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dst.getId(), 0);
  glViewport(0,0,dstRes.x,dstRes.y);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  bloom1Shader_.use();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, src.getId());
  glUniform1i(bloom1Shader_.getUniformLocation("inTex"), 0);

  glm::vec2 offset = glm::vec2(0.0);
  if (horizontal)
  {
    offset.x = 1.2/srcRes.x;
  } else {
    offset.y = 1.2/srcRes.y;
  }

  glUniform2f(bloom1Shader_.getUniformLocation("offset"), offset.x, offset.y);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadBuffer_.getId());
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);
}

void Renderer::renderScreen(const Texture& tex)
{
  // First we're going to composite our frame with the previous frame
  // We start by setting up the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, genericFb_.getId());
  glFramebufferTexture(
    GL_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT0,
    renderPages_[curBuf_].getId(),
    0);

  // Set up the shader
  glViewport(0,0,GAME_WIDTH,GAME_HEIGHT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ntscShader_.use();

  // Use the current frame texture, nearest neighbor and clamped to edge
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex.getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glUniform1i(ntscShader_.getUniformLocation("curFrameSampler"), 0);

  // Use the previous frame composite texture, nearest neighbor and clamped to
  // edge
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, renderPages_[(curBuf_ + 1) % 2].getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glUniform1i(ntscShader_.getUniformLocation("prevFrameSampler"), 1);

  // Load the NTSC artifact texture
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, artifactsTex_.getId());
  glUniform1i(ntscShader_.getUniformLocation("NTSCArtifactSampler"), 2);
  glUniform1f(ntscShader_.getUniformLocation("NTSCLerp"), curBuf_ * 1.0);

  // Change the 0.0 to a 1.0 or a 10.0 for a glitchy effect!
  glUniform1f(ntscShader_.getUniformLocation("Tuning_NTSC"), 0.0);

  // Render our composition
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadBuffer_.getId());
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);

  // We're going to render the screen now
  glBindFramebuffer(GL_FRAMEBUFFER, bloomFb_.getId());
  glFramebufferTexture(
    GL_FRAMEBUFFER,
    GL_COLOR_ATTACHMENT1,
    preBloomTex_.getId(),
    0);

  glViewport(0,0,width_,height_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  finalShader_.use();

  // Use the composited frame texture, linearly filtered and filling in black
  // for the border
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderPages_[curBuf_].getId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);

  float borderColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glGenerateMipmap(GL_TEXTURE_2D);
  glUniform1i(finalShader_.getUniformLocation("rendertex"), 0);

  // Use the scanlines texture
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, scanlinesTex_.getId());
  glUniform1i(finalShader_.getUniformLocation("scanlinestex"), 1);

  // Initialize the MVP matrices
  glm::mat4 p_matrix = glm::perspective(
    glm::radians(25.0f),
    static_cast<float>(width_) / static_cast<float>(height_),
    0.1f,
    100.0f);

  glm::mat4 v_matrix = glm::lookAt(
    glm::vec3(3.75,0,0), // Camera
    glm::vec3(0,0,0),    // Center
    glm::vec3(0,1,0));   // Up

  glm::mat4 m_matrix = glm::mat4(1.0);
  glm::mat4 mvp_matrix = p_matrix * v_matrix * m_matrix;

  glUniformMatrix4fv(
    finalShader_.getUniformLocation("MVP"),
    1,
    GL_FALSE,
    &mvp_matrix[0][0]);

  glUniformMatrix4fv(
    finalShader_.getUniformLocation("worldMat"),
    1,
    GL_FALSE,
    &m_matrix[0][0]);

  glUniform2f(finalShader_.getUniformLocation("resolution"), width_, height_);
  glUniform1f(finalShader_.getUniformLocation("iGlobalTime"), glfwGetTime());

  glUniform3f(
    finalShader_.getUniformLocation("frameColor"),
    0.76f,
    0.78f,
    0.81f);

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, monitor_.getVertexBufferId());
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, monitor_.getNormalBufferId());
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, monitor_.getUvBufferId());
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, monitor_.getIndexBufferId());
  glDrawElements(
    GL_TRIANGLES,
    monitor_.getIndexCount(),
    GL_UNSIGNED_SHORT,
    nullptr);

  glDisableVertexAttribArray(2);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(0);

  // First pass of bloom!
  glm::vec2 bufferSize = glm::vec2(width_, height_);

  bloomPass1(
    preBloomTex_,
    bloomPassTex1_,
    true,
    bufferSize,
    bufferSize / 4.0f);

  bloomPass1(
    bloomPassTex1_,
    bloomPassTex2_,
    false,
    bufferSize / 4.0f,
    bufferSize / 4.0f);

  // Do the second pass of bloom and render to screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, width_, height_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  bloom2Shader_.use();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, preBloomTex_.getId());
  glUniform1i(bloom2Shader_.getUniformLocation("clearTex"), 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, bloomPassTex2_.getId());
  glUniform1i(bloom2Shader_.getUniformLocation("blurTex"), 1);

  glUniform1f(bloom2Shader_.getUniformLocation("iGlobalTime"), glfwGetTime());

  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, quadBuffer_.getId());
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glDisableVertexAttribArray(0);

  glfwSwapBuffers(window_.getHandle());

  curBuf_ = (curBuf_ + 1) % 2;
}
