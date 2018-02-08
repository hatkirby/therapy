#include "animation.h"

AnimationSet::AnimationSet(
  Texture texture,
  int frameWidth,
  int frameHeight,
  int framesAcross) :
    texture_(std::move(texture)),
    frameWidth_(frameWidth),
    frameHeight_(frameHeight),
    framesAcross_(framesAcross)
{
}

void AnimationSet::emplaceAnimation(
  std::string animation,
  size_t firstFrame,
  size_t numFrames,
  size_t delay)
{
  animations_.emplace(
    std::piecewise_construct,
    std::make_tuple(animation),
    std::make_tuple(firstFrame, numFrames, delay));
}

Rectangle AnimationSet::getFrameRect(int frame) const
{
  return {
    frameWidth_ * (frame % framesAcross_),
    frameHeight_ * (frame / framesAcross_),
    frameWidth_,
    frameHeight_
  };
}
