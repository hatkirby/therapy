#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

#ifndef RENDERER_H
#define RENDERER_H

struct Rectangle {
  int x;
  int y;
  int w;
  int h;
  
  Rectangle() {};
  
  Rectangle(int m_x, int m_y, int m_w, int m_h)
  {
    x = m_x;
    y = m_y;
    w = m_w;
    h = m_h;
  }
};

struct Texture {
  GLuint texID;
  int width;
  int height;
};

GLFWwindow* initRenderer();
void destroyRenderer();
Texture* createTexture(int width, int height);
void destroyTexture(Texture* tex);
Texture* loadTextureFromBMP(char* filename);
void saveTextureToBMP(Texture* tex, char* filename);
void fillTexture(Texture* tex, Rectangle* loc, int r, int g, int b);
void blitTexture(Texture* srctex, Texture* dsttex, Rectangle* srcrect, Rectangle* dstrect);
void renderScreen(Texture* tex);

#endif
