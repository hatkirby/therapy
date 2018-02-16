#include "texture.h"
#include <stdexcept>
#include "renderer.h"
#include "util.h"

// include stb_image
#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#include "stb_image.h"

Texture::Texture(
  int width,
  int height) :
    width_(width),
    height_(height)
{
  if (!Renderer::isSingletonInitialized())
  {
    throw std::logic_error("Renderer needs to be initialized");
  }

  glBindTexture(GL_TEXTURE_2D, texture_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    width_,
    height_,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
}

Texture::Texture(const char* filename)
{
  if (!Renderer::isSingletonInitialized())
  {
    throw std::logic_error("Renderer needs to be initialized");
  }

  glBindTexture(GL_TEXTURE_2D, texture_.getId());
  unsigned char* data = stbi_load(filename, &width_, &height_, 0, 4);
  flipImageData(data, width_, height_, 4);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    width_,
    height_,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    data);

  stbi_image_free(data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Texture::Texture(
  const Texture& tex) :
    width_(tex.width_),
    height_(tex.height_)
{
  if (!Renderer::isSingletonInitialized())
  {
    throw std::logic_error("Renderer needs to be initialized");
  }

  unsigned char* data = new unsigned char[4 * width_ * height_];
  glBindTexture(GL_TEXTURE_2D, tex.getId());
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, texture_.getId());
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    width_,
    height_,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

  delete[] data;
}

Texture::Texture(Texture&& tex) : Texture(0, 0)
{
  swap(*this, tex);
}

Texture& Texture::operator= (Texture tex)
{
  swap(*this, tex);

  return *this;
}

void swap(Texture& tex1, Texture& tex2)
{
  std::swap(tex1.width_, tex2.width_);
  std::swap(tex1.height_, tex2.height_);
  std::swap(tex1.texture_, tex2.texture_);
}

Rectangle Texture::entirety() const
{
  return {0, 0, width_, height_};
}
