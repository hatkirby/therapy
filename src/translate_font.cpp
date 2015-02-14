#include "renderer.h"
#include <cstdio>

#define min(x,y) ((x) > (y) ? (y) : (x))

int main()
{
  initRenderer();
  
  Texture* palette = createTexture(128, 128);
  fillTexture(palette, NULL, 0, 0, 0);
  
  Texture* wrong = loadTextureFromBMP("../res/arcadepix_regular_8.bmp");
  
  FILE* desc = fopen("../res/arcadepix_regular_8.sfl", "r");
  while (!feof(desc))
  {
    int ch;
    Rectangle srcRect;
    
    fscanf(desc, "%d %d %d %d %d %*d %*d %*d \n", &ch, &(srcRect.x), &(srcRect.y), &(srcRect.w), &(srcRect.h));
    
    Rectangle dstRect(ch % 16 * 8, ch / 16 * 8 + (8 - min(srcRect.h,8)), min(srcRect.w, 8), min(srcRect.h, 8));
    blitTexture(wrong, palette, &srcRect, &dstRect);
  }
  
  fclose(desc);
  
  saveTextureToBMP(palette, "tex.bmp");
  
  destroyRenderer();
}
