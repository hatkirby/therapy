#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifndef RENDERER_H
#define RENDERER_H

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
    Texture(Texture& tex);
    Texture(Texture&& tex);
    ~Texture();
    Texture& operator= (Texture tex);
    friend void swap(Texture& tex1, Texture& tex2);
    void fill(Rectangle loc, int r, int g, int b);
    void blit(Texture& src, Rectangle srcrect, Rectangle dstrect);
    void renderScreen();
    Rectangle entirety();
    
  private:
    GLuint texID;
    int width;
    int height;
};

GLFWwindow* initRenderer();
void destroyRenderer();

#endif
