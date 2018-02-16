#ifndef RENDERER_H
#define RENDERER_H

#include "gl.h"
#include "wrappers.h"
#include "mesh.h"
#include "shader.h"
#include <glm/glm.hpp>

class Texture;
struct Rectangle;

class Renderer {
public:

  class Window {
  public:

    Window();

    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

    ~Window();

    inline GLFWwindow* getHandle()
    {
      return window_;
    }

  private:

    GLFWwindow* window_;
  };

  static inline bool isSingletonInitialized()
  {
    return singletonInitialized_;
  }

  Renderer();

  Renderer(const Renderer& other) = delete;
  Renderer& operator=(const Renderer& other) = delete;

  ~Renderer();

  inline Window& getWindow()
  {
    return window_;
  }

  void fill(
    Texture& tex,
    Rectangle loc,
    int r,
    int g,
    int b);

  void blit(
    const Texture& src,
    Texture& dst,
    Rectangle srcrect,
    Rectangle dstrect,
    double alpha = 1.0);

  void renderScreen(const Texture& tex);

private:

  friend void setFramebufferSize(GLFWwindow* w, int width, int height);

  void initializeFramebuffers();

  void bloomPass1(
    const GLTexture& src,
    GLTexture& dst,
    bool horizontal,
    glm::vec2 srcRes,
    glm::vec2 dstRes);

  static bool singletonInitialized_;

  Window window_;
  GLVertexArray vao_;

  GLFramebuffer genericFb_;
  GLFramebuffer bloomFb_;
  GLRenderbuffer bloomDepth_;

  GLTexture renderPages_[2];
  GLTexture preBloomTex_;
  GLTexture bloomPassTex1_;
  GLTexture bloomPassTex2_;

  Mesh monitor_;
  GLBuffer quadBuffer_;

  GLTexture artifactsTex_;
  GLTexture scanlinesTex_;

  Shader ntscShader_;
  Shader finalShader_;
  Shader blitShader_;
  Shader fillShader_;
  Shader bloom1Shader_;
  Shader bloom2Shader_;

  size_t curBuf_ = 0;
  int width_;
  int height_;
};

#endif
