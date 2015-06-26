#include "controlling.h"
#include "game.h"
#include "components/controllable.h"
#include "components/ponderable.h"
#include "components/animatable.h"
#include "components/droppable.h"
#include "direction.h"
#include "muxer.h"
#include "consts.h"

void ControllingSystem::tick(double dt)
{
  while (!actions.empty())
  {
    int key = actions.front().first;
    int action = actions.front().second;
    
    auto entities = game.getEntityManager().getEntitiesWithComponents<ControllableComponent, PonderableComponent, AnimatableComponent, DroppableComponent>();
    for (auto entity : entities)
    {
      auto& controllable = game.getEntityManager().getComponent<ControllableComponent>(entity);
      
      if (action == GLFW_PRESS)
      {
        if (key == controllable.getLeftKey())
        {
          controllable.setHoldingLeft(true);

          if (!controllable.isFrozen())
          {
            walkLeft(entity);
          }
        } else if (key == controllable.getRightKey())
        {
          controllable.setHoldingRight(true);
      
          if (!controllable.isFrozen())
          {
            walkRight(entity);
          }
        } else if (key == controllable.getJumpKey())
        {
          if (!controllable.isFrozen())
          {
            jump(entity);
          }
        } else if (key == controllable.getDropKey())
        {
          if (!controllable.isFrozen())
          {
            drop(entity, true);
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
              walkRight(entity);
            } else {
              stopWalking(entity);
            }
          }
        } else if (key == controllable.getRightKey())
        {
          controllable.setHoldingRight(false);
      
          if (!controllable.isFrozen())
          {
            if (controllable.isHoldingRight())
            {
              walkLeft(entity);
            } else {
              stopWalking(entity);
            }
          }
        } else if (key == controllable.getDropKey())
        {
          if (!controllable.isFrozen())
          {
            drop(entity, false);
          }
        } else if (key == controllable.getJumpKey())
        {
          if (!controllable.isFrozen())
          {
            stopJumping(entity);
          }
        }
      }
    }
    
    actions.pop();
  }
}

void ControllingSystem::input(int key, int action)
{
  actions.push(std::make_pair(key, action));
}

void ControllingSystem::walkLeft(int entity)
{
  auto& ponderable = game.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& animatable = game.getEntityManager().getComponent<AnimatableComponent>(entity);
  
  ponderable.setVelocityX(-90);
  
  animatable.setDirection(Direction::Left);
  animatable.setWalking(true);
}

void ControllingSystem::walkRight(int entity)
{
  auto& ponderable = game.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& animatable = game.getEntityManager().getComponent<AnimatableComponent>(entity);
  
  ponderable.setVelocityX(90);

  animatable.setDirection(Direction::Right);
  animatable.setWalking(true);
}

void ControllingSystem::stopWalking(int entity)
{
  auto& ponderable = game.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& animatable = game.getEntityManager().getComponent<AnimatableComponent>(entity);
  
  ponderable.setVelocityX(0);
  
  animatable.setWalking(false);
}

void ControllingSystem::jump(int entity)
{
  auto& ponderable = game.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& animatable = game.getEntityManager().getComponent<AnimatableComponent>(entity);
  
  playSound("res/Randomize87.wav", 0.25);

  ponderable.setVelocityY(JUMP_VELOCITY(TILE_HEIGHT*4.5, 0.3));
  ponderable.setAccelY(JUMP_GRAVITY(TILE_HEIGHT*4.5, 0.3));
  
  animatable.setJumping(true);
}

void ControllingSystem::stopJumping(int entity)
{
  auto& ponderable = game.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& animatable = game.getEntityManager().getComponent<AnimatableComponent>(entity);
  
  ponderable.setAccelY(JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233));
  animatable.setJumping(false);
}

void ControllingSystem::drop(int entity, bool start)
{
  auto& animatable = game.getEntityManager().getComponent<AnimatableComponent>(entity);
  auto& droppable = game.getEntityManager().getComponent<DroppableComponent>(entity);
  
  droppable.setDroppable(start);
  animatable.setCrouching(start);
}
