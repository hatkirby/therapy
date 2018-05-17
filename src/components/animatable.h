#ifndef SPRITE_RENDERABLE_H_D3AACBBF
#define SPRITE_RENDERABLE_H_D3AACBBF

#include "component.h"
#include "animation.h"
#include <string>

class AnimatableComponent : public Component {
public:

  /**
   * Constructor for initializing the animation set, because it is not default
   * constructible.
   */
  AnimatableComponent(
    AnimationSet animationSet) :
      animationSet(std::move(animationSet))
  {
  }

  /**
   * The animation set that this entity will use -- an object describing the
   * different animations that can be used to render the entity.
   *
   * @managed_by RealizingSystem
   */
  AnimationSet animationSet;

  /**
   * The name of the currently active animation.
   *
   * @managed_by AnimatingSystem
   */
  std::string animation;

  /**
   * For prototypes, the name of the original animation.
   *
   * @managed_by RealizingSystem
   */
  std::string origAnimation;

  /**
   * Helper method for accessing the currently active animation.
   */
  inline const Animation& getAnimation() const
  {
    return animationSet.getAnimation(animation);
  }

  /**
   * The frame of animation that is currently being rendered.
   *
   * @managed_by AnimatingSystem
   */
  size_t frame = 0;

  /**
   * The amount of time (in game frames) before the animation is advanced.
   *
   * @managed_by AnimatingSystem
   */
  size_t countdown = 0;

  /**
   * This option allows to give the sprite a "flickering" effect (as in, it is
   * not rendered in some frames).
   */
  bool flickering = false;

  /**
   * Used for the flickering effect.
   *
   * @managed_by AnimatingSystem
   */
  size_t flickerTimer = 0;

  /**
   * If enabled, this will prevent the sprite's animation from progressing (but
   * will not affect things such as placement on screen and flickering).
   */
  bool frozen = false;

  /**
   * If this flag is disabled, the entity will be ignored by the animating
   * system.
   *
   * @managed_by RealizingSystem
   */
  bool active = false;
};

#endif /* end of include guard: SPRITE_RENDERABLE_H_D3AACBBF */
