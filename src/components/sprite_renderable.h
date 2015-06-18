#ifndef SPRITE_RENDERABLE_H_D3AACBBF
#define SPRITE_RENDERABLE_H_D3AACBBF

#include "component.h"
#include "renderer.h"

class SpriteRenderableComponent : public Component {
  public:
    SpriteRenderableComponent(const char* filename, int frame_width, int frame_height, int frames_across);
    
    int getFrame() const;
    void setFrame(int frame);
    
    const Texture& getTexture() const;
    Rectangle getFrameRect() const;
    
  private:
    Texture texture;
    int frame_width;
    int frame_height;
    int frames_across;
    int frame = 0;
};

#endif /* end of include guard: SPRITE_RENDERABLE_H_D3AACBBF */
