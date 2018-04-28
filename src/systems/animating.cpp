#include "animating.h"
#include "game.h"
#include "components/animatable.h"
#include "components/transformable.h"

void AnimatingSystem::tick(double)
{
  std::set<id_type> spriteEntities =
    game_.getEntityManager().getEntitiesWithComponents<AnimatableComponent>();

  for (id_type entity : spriteEntities)
  {
    auto& sprite = game_.getEntityManager().
      getComponent<AnimatableComponent>(entity);

    if (sprite.active)
    {
      if (!sprite.frozen)
      {
        sprite.countdown++;
      }

      const Animation& anim = sprite.getAnimation();
      if (sprite.countdown >= anim.getDelay())
      {
        sprite.frame++;
        sprite.countdown = 0;

        if (sprite.frame >= anim.getFirstFrame() + anim.getNumFrames())
        {
          sprite.frame = anim.getFirstFrame();
        }
      }

      if (sprite.flickering)
      {
        sprite.flickerTimer = (sprite.flickerTimer + 1) % 6;
      }
    }
  }
}

void AnimatingSystem::render(Texture& texture)
{
  std::set<id_type> spriteEntities =
    game_.getEntityManager().getEntitiesWithComponents<
      AnimatableComponent,
      TransformableComponent>();

  for (id_type entity : spriteEntities)
  {
    auto& sprite = game_.getEntityManager().
      getComponent<AnimatableComponent>(entity);

    if (sprite.active)
    {
      auto& transform = game_.getEntityManager().
        getComponent<TransformableComponent>(entity);

      double alpha = 1.0;
      if (sprite.flickering && (sprite.flickerTimer < 3))
      {
        alpha = 0.0;
      }

      Rectangle dstrect {
        static_cast<int>(transform.x),
        static_cast<int>(transform.y),
        transform.w,
        transform.h};

      const AnimationSet& aset = sprite.animationSet;
      game_.getRenderer().blit(
        aset.getTexture(),
        texture,
        aset.getFrameRect(sprite.frame),
        dstrect,
        alpha);
    }
  }
}

void AnimatingSystem::initPrototype(id_type entity)
{
  auto& sprite = game_.getEntityManager().
    getComponent<AnimatableComponent>(entity);

  startAnimation(entity, sprite.origAnimation);

  sprite.countdown = 0;
  sprite.flickering = false;
  sprite.flickerTimer = 0;
  sprite.frozen = false;
}

void AnimatingSystem::startAnimation(id_type entity, std::string animation)
{
  auto& sprite = game_.getEntityManager().
    getComponent<AnimatableComponent>(entity);

  sprite.animation = std::move(animation);
  sprite.frame = sprite.getAnimation().getFirstFrame();
}
