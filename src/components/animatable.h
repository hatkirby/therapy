#ifndef SPRITE_RENDERABLE_H_D3AACBBF
#define SPRITE_RENDERABLE_H_D3AACBBF

#include "component.h"
#include "animation.h"
#include <string>

class AnimatableComponent : public Component {
public:

  AnimatableComponent(
    AnimationSet animationSet,
    std::string animation) :
      animationSet_(std::move(animationSet)),
      animation_(std::move(animation))
  {
  }

  inline size_t getFrame() const
  {
    return frame_;
  }

  inline void setFrame(size_t v)
  {
    frame_ = v;
  }

  inline size_t getCountdown() const
  {
    return countdown_;
  }

  inline void setCountdown(size_t v)
  {
    countdown_ = v;
  }

  inline const AnimationSet& getAnimationSet() const
  {
    return animationSet_;
  }

  inline const Animation& getAnimation() const
  {
    return animationSet_.getAnimation(animation_);
  }

  inline void setAnimation(std::string animation)
  {
    animation_ = std::move(animation);
  }

private:

  AnimationSet animationSet_;
  std::string animation_;
  size_t frame_ = 0;
  size_t countdown_ = 0;
};

#endif /* end of include guard: SPRITE_RENDERABLE_H_D3AACBBF */
