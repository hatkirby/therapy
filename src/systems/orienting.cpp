#include "orienting.h"
#include "game.h"
#include "components/orientable.h"
#include "components/ponderable.h"
#include "systems/animating.h"
#include "consts.h"
#include "muxer.h"

void OrientingSystem::tick(double)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    OrientableComponent,
    PonderableComponent>();

  for (id_type entity : entities)
  {
    auto& orientable = game_.getEntityManager().
      getComponent<OrientableComponent>(entity);

    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    switch (orientable.getWalkState())
    {
      case OrientableComponent::WalkState::still:
      {
        ponderable.velX = 0.0;

        break;
      }

      case OrientableComponent::WalkState::left:
      {
        ponderable.velX = -WALK_SPEED;

        break;
      }

      case OrientableComponent::WalkState::right:
      {
        ponderable.velX = WALK_SPEED;

        break;
      }
    }

    if (orientable.isJumping() && (ponderable.velY > 0))
    {
      orientable.setJumping(false);
    }
  }
}

void OrientingSystem::moveLeft(id_type entity)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  orientable.setFacingRight(false);
  orientable.setWalkState(OrientableComponent::WalkState::left);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();
  if (ponderable.grounded)
  {
    animating.startAnimation(entity, "walkingLeft");
  } else {
    animating.startAnimation(entity, "stillLeft");
  }
}

void OrientingSystem::moveRight(id_type entity)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  orientable.setFacingRight(true);
  orientable.setWalkState(OrientableComponent::WalkState::right);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();
  if (ponderable.grounded)
  {
    animating.startAnimation(entity, "walkingRight");
  } else {
    animating.startAnimation(entity, "stillRight");
  }
}

void OrientingSystem::stopWalking(id_type entity)
{
  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  orientable.setWalkState(OrientableComponent::WalkState::still);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();

  if (orientable.isFacingRight())
  {
    animating.startAnimation(entity, "stillRight");
  } else {
    animating.startAnimation(entity, "stillLeft");
  }
}

void OrientingSystem::jump(id_type entity)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  if (ponderable.grounded)
  {
    auto& orientable = game_.getEntityManager().
      getComponent<OrientableComponent>(entity);

    orientable.setJumping(true);

    playSound("res/Randomize87.wav", 0.25);

    ponderable.velY = JUMP_VELOCITY;
    ponderable.accelY = JUMP_GRAVITY;

    auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();
    if (orientable.isFacingRight())
    {
      animating.startAnimation(entity, "stillRight");
    } else {
      animating.startAnimation(entity, "stillLeft");
    }
  }
}

void OrientingSystem::stopJumping(id_type entity)
{
  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  if (orientable.isJumping())
  {
    orientable.setJumping(false);

    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    ponderable.accelY = NORMAL_GRAVITY;
  }
}

void OrientingSystem::land(id_type entity)
{
  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();

  switch (orientable.getWalkState())
  {
    case OrientableComponent::WalkState::still:
    {
      if (orientable.isFacingRight())
      {
        animating.startAnimation(entity, "stillRight");
      } else {
        animating.startAnimation(entity, "stillLeft");
      }

      break;
    }

    case OrientableComponent::WalkState::left:
    {
      animating.startAnimation(entity, "walkingLeft");

      break;
    }

    case OrientableComponent::WalkState::right:
    {
      animating.startAnimation(entity, "walkingRight");

      break;
    }
  }
}

void OrientingSystem::startFalling(id_type entity)
{
  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  auto& animating = game_.getSystemManager().getSystem<AnimatingSystem>();

  if (orientable.isFacingRight())
  {
    animating.startAnimation(entity, "stillRight");
  } else {
    animating.startAnimation(entity, "stillLeft");
  }
}

void OrientingSystem::drop(id_type entity)
{
  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  if (ponderable.grounded
    && (orientable.getDropState() == OrientableComponent::DropState::none))
  {
    orientable.setDropState(OrientableComponent::DropState::ready);
  }
}

void OrientingSystem::stopDropping(id_type entity)
{
  auto& orientable = game_.getEntityManager().
    getComponent<OrientableComponent>(entity);

  if (orientable.getDropState() == OrientableComponent::DropState::ready)
  {
    orientable.setDropState(OrientableComponent::DropState::none);
  }
}
