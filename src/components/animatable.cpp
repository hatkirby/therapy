#include "animatable.h"

AnimatableComponent::AnimatableComponent(const char* filename, int frame_width, int frame_height, int frames_across)
  : texture(filename), frame_width(frame_width), frame_height(frame_height), frames_across(frames_across)
{
  
}

int AnimatableComponent::getFrame() const
{
  return frame;
}

void AnimatableComponent::setFrame(int frame)
{
  this->frame = frame;
}

const Texture& AnimatableComponent::getTexture() const
{
  return texture;
}

Rectangle AnimatableComponent::getFrameRect() const
{
  return {frame_width * (frame % frames_across), frame_height * (frame / frames_across), frame_width, frame_height};
}
