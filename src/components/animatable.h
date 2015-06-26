#ifndef SPRITE_RENDERABLE_H_D3AACBBF
#define SPRITE_RENDERABLE_H_D3AACBBF

#include "component.h"
#include "renderer.h"
#include "direction.h"

class AnimatableComponent : public Component {
  public:
    AnimatableComponent(const char* filename, int frame_width, int frame_height, int frames_across);
    
    int getFrame() const;
    void setFrame(int frame);
    
    const Texture& getTexture() const;
    Rectangle getFrameRect() const;
    
    void setDirection(Direction dir) {};
    void setWalking(bool w) {};
    void setJumping(bool w) {};
    void setCrouching(bool w) {};
    
  private:
    Texture texture;
    int frame_width;
    int frame_height;
    int frames_across;
    int frame = 0;
};

#endif /* end of include guard: SPRITE_RENDERABLE_H_D3AACBBF */
