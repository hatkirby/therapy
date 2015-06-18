#include "rendering.h"
#include "entity_manager.h"
#include "components/sprite_renderable.h"
#include "components/transformable.h"

void RenderingSystem::tick(EntityManager& manager, float dt)
{
  texture.fill(texture.entirety(), 0, 0, 0);
  
  std::set<int> spriteEntities = manager.getEntitiesWithComponents<SpriteRenderableComponent, TransformableComponent>();
  for (int entity : spriteEntities)
  {
    auto& sprite = manager.getComponent<SpriteRenderableComponent>(entity);
    auto& transform = manager.getComponent<TransformableComponent>(entity);
    Rectangle dstrect {(int) transform.getX(), (int) transform.getY(), transform.getW(), transform.getH()};
    
    texture.blit(sprite.getTexture(), sprite.getFrameRect(), dstrect);
  }
  
  texture.renderScreen();
}
