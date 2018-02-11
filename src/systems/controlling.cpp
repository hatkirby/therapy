#include "controlling.h"
#include "game.h"
#include "components/controllable.h"
#include "components/ponderable.h"
#include "components/animatable.h"
#include "components/droppable.h"
#include "components/orientable.h"
#include "systems/animating.h"
#include "direction.h"
#include "muxer.h"
#include "consts.h"

void ControllingSystem::tick(double)
{
  while (!actions_.empty())
  {
    int key = actions_.front().first;
    int action = actions_.front().second;

    auto entities = game_.getEntityManager().getEntitiesWithComponents<
      ControllableComponent,
      PonderableComponent,
      AnimatableComponent,
      DroppableComponent,
      OrientableComponent>();

    for (auto entity : entities)
    {
      auto& controllable = game_.getEntityManager().
        getComponent<ControllableComponent>(entity);

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
            if (controllable.isHoldingLeft())
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

    actions_.pop();
  }
}

void ControllingSystem::input(int key, int action)
{
  actions_.push(std::make_pair(key, action));
}

void ControllingSystem::walkLeft(id_type entity)
{
  auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& orientable = game_.getEntityManager().getComponent<OrientableComponent>(entity);

  orientable.setFacingRight(false);
  ponderable.setVelocityX(-90);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();

  if (ponderable.getState() == PonderableComponent::State::grounded)
  {
    animating.startAnimation(entity, "walkingLeft");
  } else {
    animating.startAnimation(entity, "stillLeft");
  }
}

void ControllingSystem::walkRight(id_type entity)
{
  auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& orientable = game_.getEntityManager().getComponent<OrientableComponent>(entity);

  orientable.setFacingRight(true);
  ponderable.setVelocityX(90);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();

  if (ponderable.getState() == PonderableComponent::State::grounded)
  {
    animating.startAnimation(entity, "walkingRight");
  } else {
    animating.startAnimation(entity, "stillRight");
  }
}

void ControllingSystem::stopWalking(id_type entity)
{
  auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);
  auto& orientable = game_.getEntityManager().getComponent<OrientableComponent>(entity);

  ponderable.setVelocityX(0);

  if (ponderable.getState() == PonderableComponent::State::grounded)
  {
    auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();

    if (orientable.isFacingRight())
    {
      animating.startAnimation(entity, "stillRight");
    } else {
      animating.startAnimation(entity, "stillLeft");
    }
  }
}

void ControllingSystem::jump(id_type entity)
{
  auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);

  if (ponderable.getState() == PonderableComponent::State::grounded)
  {
    playSound("res/Randomize87.wav", 0.25);

    ponderable.setVelocityY(JUMP_VELOCITY(TILE_HEIGHT*4.5, 0.3));
    ponderable.setAccelY(JUMP_GRAVITY(TILE_HEIGHT*4.5, 0.3));
    ponderable.setState(PonderableComponent::State::jumping);
  }
}

void ControllingSystem::stopJumping(id_type entity)
{
  auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);

  if (ponderable.getState() == PonderableComponent::State::jumping)
  {
    ponderable.setAccelY(JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233));
    ponderable.setState(PonderableComponent::State::falling);
  }
}

void ControllingSystem::drop(id_type entity, bool start)
{
  auto& droppable = game_.getEntityManager().getComponent<DroppableComponent>(entity);
  auto& ponderable = game_.getEntityManager().getComponent<PonderableComponent>(entity);

  if (start && (ponderable.getState() == PonderableComponent::State::grounded))
  {
    ponderable.setState(PonderableComponent::State::dropping);
  } else if ((!start) && (ponderable.getState() == PonderableComponent::State::dropping))
  {
    ponderable.setState(PonderableComponent::State::grounded);
  }
  droppable.setDroppable(start);
}
