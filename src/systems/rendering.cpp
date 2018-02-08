#include "rendering.h"
#include "game.h"
#include "components/animatable.h"
#include "components/transformable.h"

void RenderingSystem::tick(double)
{
  texture_.fill(texture_.entirety(), 0, 0, 0);

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
    texture_.blit(
      aset.getTexture(),
      aset.getFrameRect(sprite.getFrame()),
      dstrect);
  }

  texture_.renderScreen();
}
