#include "sprite_renderable.h"

SpriteRenderableComponent::SpriteRenderableComponent(const char* filename, int frame_width, int frame_height, int frames_across)
  : texture(filename), frame_width(frame_width), frame_height(frame_height), frames_across(frames_across)
{
  
}

int SpriteRenderableComponent::getFrame() const
{
  return frame;
}

void SpriteRenderableComponent::setFrame(int frame)
{
  this->frame = frame;
}

const Texture& SpriteRenderableComponent::getTexture() const
{
  return texture;
}

Rectangle SpriteRenderableComponent::getFrameRect() const
{
  return {frame_width * (frame % frames_across), frame_height * (frame / frames_across), frame_width, frame_height};
}
