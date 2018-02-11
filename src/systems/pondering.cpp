#include "pondering.h"
#include "game.h"
#include "components/ponderable.h"
#include "components/transformable.h"
#include "components/droppable.h"
#include "consts.h"

void PonderingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    PonderableComponent,
    TransformableComponent>();

  auto maps = game_.getEntityManager().getEntitiesWithComponents<
    MappableComponent>();

  for (id_type entity : entities)
  {
    auto& transformable = game_.getEntityManager().
      getComponent<TransformableComponent>(entity);

    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    // Accelerate
    ponderable.setVelocityX(
      ponderable.getVelocityX() + ponderable.getAccelX() * dt);

    ponderable.setVelocityY(
      ponderable.getVelocityY() + ponderable.getAccelY() * dt);

    const double oldX = transformable.getX();
    const double oldY = transformable.getY();
    const double oldRight = oldX + transformable.getW();
    const double oldBottom = oldY + transformable.getH();

    double newX = oldX + ponderable.getVelocityX() * dt;
    double newY = oldY + ponderable.getVelocityY() * dt;

    if (ponderable.getVelocityY() > 0.0)
    {
      ponderable.setState(PonderableComponent::State::falling);
    }

    for (id_type mapEntity : maps)
    {
      auto& mappable = game_.getEntityManager().
        getComponent<MappableComponent>(mapEntity);

      if (newX < oldX)
      {
        for (auto it = mappable.getLeftBoundaries().lower_bound(oldX);
          (it != std::end(mappable.getLeftBoundaries())) && (it->first >= newX);
          it++)
        {
          if ((oldBottom > it->second.getLower())
            && (oldY < it->second.getUpper()))
          {
            // We have a collision!
            processCollision(
              entity,
              Direction::left,
              newX,
              newY,
              it->first,
              it->second.getType());

            break;
          }
        }
      } else if (newX > oldX)
      {
        for (auto it = mappable.getRightBoundaries().lower_bound(oldRight);
          (it != std::end(mappable.getRightBoundaries()))
            && (it->first <= (newX + transformable.getW()));
          it++)
        {
          if ((oldBottom > it->second.getLower())
            && (oldY < it->second.getUpper()))
          {
            // We have a collision!
            processCollision(
              entity,
              Direction::right,
              newX,
              newY,
              it->first,
              it->second.getType());

            break;
          }
        }
      }

      if (newY < oldY)
      {
        for (auto it = mappable.getUpBoundaries().lower_bound(oldY);
          (it != std::end(mappable.getUpBoundaries())) && (it->first >= newY);
          it++)
        {
          if ((oldRight > it->second.getLower())
            && (oldX < it->second.getUpper()))
          {
            // We have a collision!
            processCollision(
              entity,
              Direction::up,
              newX,
              newY,
              it->first,
              it->second.getType());

            break;
          }
        }
      } else if (newY > oldY)
      {
        for (auto it = mappable.getDownBoundaries().lower_bound(oldBottom);
          (it != std::end(mappable.getDownBoundaries()))
            && (it->first <= (newY + transformable.getH()));
          it++)
        {
          if ((oldRight > it->second.getLower())
            && (oldX < it->second.getUpper()))
          {
            // We have a collision!
            processCollision(
              entity,
              Direction::down,
              newX,
              newY,
              it->first,
              it->second.getType());

            break;
          }
        }
      }
    }

    // Move
    transformable.setX(newX);
    transformable.setY(newY);
  }
}

void PonderingSystem::initializeBody(
  id_type entity,
  PonderableComponent::Type type)
{
  auto& ponderable = game_.getEntityManager().
    emplaceComponent<PonderableComponent>(entity, type);

  if (type == PonderableComponent::Type::freefalling)
  {
    ponderable.setAccelY(JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233));
    ponderable.setState(PonderableComponent::State::falling);
  }
}

void PonderingSystem::processCollision(
  id_type entity,
  Direction dir,
  double& newX,
  double& newY,
  int axis,
  MappableComponent::Boundary::Type type)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  switch (type)
  {
    case MappableComponent::Boundary::Type::wall:
    {
      switch (dir)
      {
        case Direction::left:
        {
          newX = axis;
          ponderable.setVelocityX(0.0);

          break;
        }

        case Direction::right:
        {
          newX = axis - transformable.getW();
          ponderable.setVelocityX(0.0);

          break;
        }

        case Direction::up:
        {
          newY = axis;
          ponderable.setVelocityY(0.0);

          break;
        }

        case Direction::down:
        {
          newY = axis - transformable.getH();
          ponderable.setVelocityY(0.0);

          if (ponderable.getState() == PonderableComponent::State::falling)
          {
            ponderable.setState(PonderableComponent::State::grounded);
          }

          break;
        }
      }

      break;
    }

    case MappableComponent::Boundary::Type::platform:
    {
      if (game_.getEntityManager().hasComponent<DroppableComponent>(entity))
      {
        auto& droppable = game_.getEntityManager().
          getComponent<DroppableComponent>(entity);

        if (droppable.isDroppable())
        {
          droppable.setDroppable(false);
        } else {
          newY = axis - transformable.getH();
          ponderable.setVelocityY(0.0);

          if (ponderable.getState() == PonderableComponent::State::falling)
          {
            ponderable.setState(PonderableComponent::State::grounded);
          }
        }
      } else {
        newY = axis - transformable.getH();
        ponderable.setVelocityY(0.0);

        if (ponderable.getState() == PonderableComponent::State::falling)
        {
          ponderable.setState(PonderableComponent::State::grounded);
        }
      }

      break;
    }

    default:
    {
      // Not yet implemented.

      break;
    }
  }
}
