#ifndef TEXTURE_H_84EC6DF6
#define TEXTURE_H_84EC6DF6

#include "wrappers.h"

struct Rectangle {
  int x;
  int y;
  int w;
  int h;
};

class Texture {
public:

  Texture(int width, int height);

  Texture(const char* file);

  Texture(const Texture& tex);

  Texture(Texture&& tex);

  Texture& operator= (Texture tex);

  friend void swap(Texture& tex1, Texture& tex2);

  Rectangle entirety() const;

  inline GLuint getId() const
  {
    return texture_.getId();
  }

  inline int getWidth() const
  {
    return width_;
  }

  inline int getHeight() const
  {
    return height_;
  }

private:

  GLTexture texture_;
  int width_;
  int height_;
};

#endif /* end of include guard: TEXTURE_H_84EC6DF6 */
