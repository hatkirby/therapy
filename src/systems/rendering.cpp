#include "rendering.h"
#include "game.h"
#include "components/animatable.h"
#include "components/transformable.h"

void RenderingSystem::tick(double dt)
{
  texture.fill(texture.entirety(), 0, 0, 0);
  
  std::set<int> spriteEntities = game.getEntityManager().getEntitiesWithComponents<AnimatableComponent, TransformableComponent>();
  for (int entity : spriteEntities)
  {
    auto& sprite = game.getEntityManager().getComponent<AnimatableComponent>(entity);
    auto& transform = game.getEntityManager().getComponent<TransformableComponent>(entity);
    Rectangle dstrect {(int) transform.getX(), (int) transform.getY(), transform.getW(), transform.getH()};
    
    texture.blit(sprite.getTexture(), sprite.getFrameRect(), dstrect);
  }
  
  texture.renderScreen();
}
