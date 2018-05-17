#include "controlling.h"
#include "game.h"
#include "components/controllable.h"
#include "components/orientable.h"
#include "systems/orienting.h"

void ControllingSystem::tick(double)
{
  while (!actions_.empty())
  {
    int key = actions_.front().first;
    int action = actions_.front().second;

    auto entities = game_.getEntityManager().getEntitiesWithComponents<
      ControllableComponent,
      OrientableComponent>();

    for (auto entity : entities)
    {
      auto& controllable = game_.getEntityManager().
        getComponent<ControllableComponent>(entity);

      auto& orienting = game_.getSystemManager().getSystem<OrientingSystem>();

      if (action == GLFW_PRESS)
      {
        if (key == controllable.getLeftKey())
        {
          controllable.setHoldingLeft(true);

          if (!controllable.isFrozen())
          {
            orienting.moveLeft(entity);
          }
        } else if (key == controllable.getRightKey())
        {
          controllable.setHoldingRight(true);

          if (!controllable.isFrozen())
          {
            orienting.moveRight(entity);
          }
        } else if (key == controllable.getJumpKey())
        {
          if (!controllable.isFrozen())
          {
            orienting.jump(entity);
          }
        } else if (key == controllable.getDropKey())
        {
          if (!controllable.isFrozen())
          {
            orienting.drop(entity);
          }
        }
      } else if (action == GLFW_RELEASE)
      {
        if (key == controllable.getLeftKey())
        {
          controllable.setHoldingLeft(false);

          if (!controllable.isFrozen())
          {
            if (controllable.isHoldingRight())
            {
              orienting.moveRight(entity);
            } else {
              orienting.stopWalking(entity);
            }
          }
        } else if (key == controllable.getRightKey())
        {
          controllable.setHoldingRight(false);

          if (!controllable.isFrozen())
          {
            if (controllable.isHoldingLeft())
            {
              orienting.moveLeft(entity);
            } else {
              orienting.stopWalking(entity);
            }
          }
        } else if (key == controllable.getDropKey())
        {
          if (!controllable.isFrozen())
          {
            orienting.stopDropping(entity);
          }
        } else if (key == controllable.getJumpKey())
        {
          if (!controllable.isFrozen())
          {
            orienting.stopJumping(entity);
          }
        }
      }
    }

    actions_.pop();
  }
}

void ControllingSystem::input(int key, int action)
{
  actions_.push(std::make_pair(key, action));
}

void ControllingSystem::freeze(id_type entity)
{
  auto& controllable = game_.getEntityManager().
    getComponent<ControllableComponent>(entity);

  controllable.setFrozen(true);
}

void ControllingSystem::unfreeze(id_type entity)
{
  auto& controllable = game_.getEntityManager().
    getComponent<ControllableComponent>(entity);

  if (controllable.isFrozen())
  {
    controllable.setFrozen(false);

    auto& orienting = game_.getSystemManager().getSystem<OrientingSystem>();

    if (controllable.isHoldingLeft())
    {
      orienting.moveLeft(entity);
    } else if (controllable.isHoldingRight())
    {
      orienting.moveRight(entity);
    }
  }
}
