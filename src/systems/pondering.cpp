#include "pondering.h"
#include <queue>
#include "game.h"
#include "components/ponderable.h"
#include "components/transformable.h"
#include "components/orientable.h"
#include "components/mappable.h"
#include "systems/orienting.h"
#include "systems/playing.h"
#include "consts.h"
#include "collision.h"

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

    if (ponderable.isFrozen())
    {
      continue;
    }

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

    bool oldGrounded = ponderable.isGrounded();
    ponderable.setGrounded(false);

    std::priority_queue<Collision> collisions;

    // Find collisions
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
            collisions.emplace(
              mapEntity,
              Direction::left,
              it->second.getType(),
              it->first,
              it->second.getLower(),
              it->second.getUpper());
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
            collisions.emplace(
              mapEntity,
              Direction::right,
              it->second.getType(),
              it->first,
              it->second.getLower(),
              it->second.getUpper());
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
            collisions.emplace(
              mapEntity,
              Direction::up,
              it->second.getType(),
              it->first,
              it->second.getLower(),
              it->second.getUpper());
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
            collisions.emplace(
              mapEntity,
              Direction::down,
              it->second.getType(),
              it->first,
              it->second.getLower(),
              it->second.getUpper());
          }
        }
      }
    }

    // Process collisions in order of priority
    while (!collisions.empty())
    {
      Collision collision = collisions.top();
      collisions.pop();

      // Make sure that they are still colliding
      if (!collision.isColliding(
        newX,
        newY,
        transformable.getW(),
        transformable.getH()))
      {
        continue;
      }

      bool touchedWall = false;
      bool stopProcessing = false;

      switch (collision.getType())
      {
        case Collision::Type::wall:
        {
          touchedWall = true;

          break;
        }

        case Collision::Type::platform:
        {
          if (game_.getEntityManager().
            hasComponent<OrientableComponent>(entity))
          {
            auto& orientable = game_.getEntityManager().
              getComponent<OrientableComponent>(entity);

            if (orientable.getDropState() !=
              OrientableComponent::DropState::none)
            {
              orientable.setDropState(OrientableComponent::DropState::active);
            } else {
              touchedWall = true;
            }
          } else {
            touchedWall = true;
          }

          break;
        }

        case Collision::Type::adjacency:
        {
          auto& mappable = game_.getEntityManager().
            getComponent<MappableComponent>(collision.getCollider());
          const Map& map = game_.getWorld().getMap(mappable.getMapId());
          auto& adj = [&] () -> const Map::Adjacent& {
              switch (collision.getDirection())
              {
                case Direction::left: return map.getLeftAdjacent();
                case Direction::right: return map.getRightAdjacent();
                case Direction::up: return map.getUpAdjacent();
                case Direction::down: return map.getDownAdjacent();
              }
            }();

          switch (adj.getType())
          {
            case Map::Adjacent::Type::wall:
            {
              touchedWall = true;

              break;
            }

            case Map::Adjacent::Type::wrap:
            {
              switch (collision.getDirection())
              {
                case Direction::left:
                {
                  newX = GAME_WIDTH + WALL_GAP - transformable.getW();

                  break;
                }

                case Direction::right:
                {
                  newX = -WALL_GAP;

                  break;
                }

                case Direction::up:
                {
                  newY = MAP_HEIGHT * TILE_HEIGHT + WALL_GAP -
                    transformable.getH();

                  break;
                }

                case Direction::down:
                {
                  newY = -WALL_GAP;

                  break;
                }
              }
            }

            case Map::Adjacent::Type::warp:
            {
              double warpX = newX;
              double warpY = newY;

              switch (collision.getDirection())
              {
                case Direction::left:
                {
                  warpX = GAME_WIDTH + WALL_GAP - transformable.getW();

                  break;
                }

                case Direction::right:
                {
                  warpX = -WALL_GAP;

                  break;
                }

                case Direction::up:
                {
                  warpY = MAP_HEIGHT * TILE_HEIGHT - transformable.getH();

                  break;
                }

                case Direction::down:
                {
                  warpY = -WALL_GAP;

                  break;
                }
              }

              game_.getSystemManager().getSystem<PlayingSystem>().
                changeMap(adj.getMapId(), warpX, warpY);

              stopProcessing = true;

              break;
            }
          }

          break;
        }

        case Collision::Type::danger:
        {
          game_.getSystemManager().getSystem<PlayingSystem>().die();

          stopProcessing = true;

          break;
        }

        default:
        {
          // Not yet implemented.

          break;
        }
      }

      if (stopProcessing)
      {
        break;
      }

      if (touchedWall)
      {
        switch (collision.getDirection())
        {
          case Direction::left:
          {
            newX = collision.getAxis();
            ponderable.setVelocityX(0.0);

            break;
          }

          case Direction::right:
          {
            newX = collision.getAxis() - transformable.getW();
            ponderable.setVelocityX(0.0);

            break;
          }

          case Direction::up:
          {
            newY = collision.getAxis();
            ponderable.setVelocityY(0.0);

            break;
          }

          case Direction::down:
          {
            newY = collision.getAxis() - transformable.getH();
            ponderable.setVelocityY(0.0);
            ponderable.setGrounded(true);

            break;
          }
        }
      }
    }

    // Move
    transformable.setX(newX);
    transformable.setY(newY);

    // Perform cleanup for orientable entites
    if (game_.getEntityManager().hasComponent<OrientableComponent>(entity))
    {
      auto& orientable = game_.getEntityManager().
        getComponent<OrientableComponent>(entity);

      // Handle changes in groundedness
      if (ponderable.isGrounded() != oldGrounded)
      {
        if (ponderable.isGrounded())
        {
          game_.getSystemManager().getSystem<OrientingSystem>().land(entity);
        } else {
          game_.getSystemManager().
            getSystem<OrientingSystem>().startFalling(entity);
        }
      }

      // Complete dropping, if necessary
      if (orientable.getDropState() == OrientableComponent::DropState::active)
      {
        orientable.setDropState(OrientableComponent::DropState::none);
      }
    }
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
    ponderable.setAccelY(NORMAL_GRAVITY);
  }
}
