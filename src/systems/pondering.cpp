#include "pondering.h"
#include "game.h"
#include "components/ponderable.h"
#include "components/transformable.h"

void PonderingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    PonderableComponent,
    TransformableComponent>();

  for (id_type entity : entities)
  {
    auto& transformable = game_.getEntityManager().getComponent<TransformableComponent>(entity);
    auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);

    // Accelerate
    ponderable.setVelocityX(ponderable.getVelocityX() + ponderable.getAccelX() * dt);
    ponderable.setVelocityY(ponderable.getVelocityY() + ponderable.getAccelY() * dt);

    // Move
    transformable.setX(transformable.getX() + ponderable.getVelocityX() * dt);
    transformable.setY(transformable.getY() + ponderable.getVelocityY() * dt);
  }
}
