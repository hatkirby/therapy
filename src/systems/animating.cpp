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

    sprite.setCountdown(sprite.getCountdown() + 1);

    const Animation& anim = sprite.getAnimation();
    if (sprite.getCountdown() >= anim.getDelay())
    {
      sprite.setFrame(sprite.getFrame() + 1);
      sprite.setCountdown(0);

      if (sprite.getFrame() >= anim.getFirstFrame() + anim.getNumFrames())
      {
        sprite.setFrame(anim.getFirstFrame());
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

    auto& transform = game_.getEntityManager().
      getComponent<TransformableComponent>(entity);

    Rectangle dstrect {
      static_cast<int>(transform.getX()),
      static_cast<int>(transform.getY()),
      transform.getW(),
      transform.getH()};

    const AnimationSet& aset = sprite.getAnimationSet();
    texture.blit(
      aset.getTexture(),
      aset.getFrameRect(sprite.getFrame()),
      dstrect);
  }
}

void AnimatingSystem::startAnimation(id_type entity, std::string animation)
{
  auto& sprite = game_.getEntityManager().
    getComponent<AnimatableComponent>(entity);

  sprite.setAnimation(animation);
  sprite.setFrame(sprite.getAnimation().getFirstFrame());
}
