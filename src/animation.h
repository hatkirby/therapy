#ifndef ANIMATION_H_74EB0901
#define ANIMATION_H_74EB0901

#include "renderer.h"
#include <string>
#include <map>
#include <stdexcept>

class Animation {
public:

  Animation(
    size_t firstFrame,
    size_t numFrames,
    size_t delay) :
      firstFrame_(firstFrame),
      numFrames_(numFrames),
      delay_(delay)
  {
  }

  inline size_t getFirstFrame() const
  {
    return firstFrame_;
  }

  inline size_t getNumFrames() const
  {
    return numFrames_;
  }

  inline size_t getDelay() const
  {
    return delay_;
  }

private:

  size_t firstFrame_;
  size_t numFrames_;
  size_t delay_;
};

class AnimationSet {
public:

  AnimationSet(
    Texture texture,
    int frameWidth,
    int frameHeight,
    int framesAcross);

  void emplaceAnimation(
    std::string animation,
    size_t firstFrame,
    size_t numFrames,
    size_t delay);

  inline const Animation& getAnimation(std::string animation) const
  {
    if (!animations_.count(animation))
    {
      throw std::invalid_argument("Animation does not exist");
    }

    return animations_.at(animation);
  }

  inline const Texture& getTexture() const
  {
    return texture_;
  }

  inline int getFrameWidth() const
  {
    return frameWidth_;
  }

  inline int getFrameHeight() const
  {
    return frameHeight_;
  }

  inline int getFramesAcross() const
  {
    return framesAcross_;
  }

  Rectangle getFrameRect(int frame) const;

private:

  std::map<std::string, Animation> animations_;
  Texture texture_;
  int frameWidth_;
  int frameHeight_;
  int framesAcross_;
};

#endif /* end of include guard: ANIMATION_H_74EB0901 */
