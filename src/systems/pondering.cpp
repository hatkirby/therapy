#include "pondering.h"
#include <queue>
#include "game.h"
#include "components/ponderable.h"
#include "components/transformable.h"
#include "components/orientable.h"
#include "components/mappable.h"
#include "components/realizable.h"
#include "components/playable.h"
#include "systems/orienting.h"
#include "systems/playing.h"
#include "systems/realizing.h"
#include "consts.h"
#include "collision.h"

void PonderingSystem::tick(double dt)
{
  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(
      game_.getSystemManager().getSystem<RealizingSystem>().getSingleton());

  id_type mapEntity = realizable.activeMap;

  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(mapEntity);

  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    PonderableComponent,
    TransformableComponent>();

  for (id_type entity : entities)
  {
    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    if (!ponderable.active || ponderable.frozen)
    {
      continue;
    }

    auto& transformable = game_.getEntityManager().
      getComponent<TransformableComponent>(entity);

    // Accelerate
    ponderable.velX += ponderable.accelX * dt;
    ponderable.velY += ponderable.accelY * dt;

    if ((ponderable.type == PonderableComponent::Type::freefalling)
      && (ponderable.velY > TERMINAL_VELOCITY))
    {
      ponderable.velY = TERMINAL_VELOCITY;
    }

    const double oldX = transformable.x;
    const double oldY = transformable.y;
    const double oldRight = oldX + transformable.w;
    const double oldBottom = oldY + transformable.h;

    double newX = oldX + ponderable.velX * dt;
    double newY = oldY + ponderable.velY * dt;

    bool oldGrounded = ponderable.grounded;
    ponderable.grounded = false;

    std::priority_queue<Collision> collisions;

    // Find collisions
    if (newX < oldX)
    {
      for (auto it = mappable.leftBoundaries.lower_bound(oldX);
        (it != std::end(mappable.leftBoundaries)) && (it->first >= newX);
        it++)
      {
        if ((oldBottom > it->second.lower)
          && (oldY < it->second.upper))
        {
          // We have a collision!
          collisions.emplace(
            mapEntity,
            Direction::left,
            it->second.type,
            it->first,
            it->second.lower,
            it->second.upper);
        }
      }
    } else if (newX > oldX)
    {
      for (auto it = mappable.rightBoundaries.lower_bound(oldRight);
        (it != std::end(mappable.rightBoundaries))
          && (it->first <= (newX + transformable.w));
        it++)
      {
        if ((oldBottom > it->second.lower)
          && (oldY < it->second.upper))
        {
          // We have a collision!
          collisions.emplace(
            mapEntity,
            Direction::right,
            it->second.type,
            it->first,
            it->second.lower,
            it->second.upper);
        }
      }
    }

    if (newY < oldY)
    {
      for (auto it = mappable.upBoundaries.lower_bound(oldY);
        (it != std::end(mappable.upBoundaries)) && (it->first >= newY);
        it++)
      {
        if ((oldRight > it->second.lower)
          && (oldX < it->second.upper))
        {
          // We have a collision!
          collisions.emplace(
            mapEntity,
            Direction::up,
            it->second.type,
            it->first,
            it->second.lower,
            it->second.upper);
        }
      }
    } else if (newY > oldY)
    {
      for (auto it = mappable.downBoundaries.lower_bound(oldBottom);
        (it != std::end(mappable.downBoundaries))
          && (it->first <= (newY + transformable.h));
        it++)
      {
        if ((oldRight > it->second.lower)
          && (oldX < it->second.upper))
        {
          // We have a collision!
          collisions.emplace(
            mapEntity,
            Direction::down,
            it->second.type,
            it->first,
            it->second.lower,
            it->second.upper);
        }
      }
    }

    // Process collisions in order of priority
    bool adjacentlyWarping = false;
    Direction adjWarpDir;
    size_t adjWarpMapId;

    while (!collisions.empty())
    {
      Collision collision = collisions.top();
      collisions.pop();

      // Make sure that they are still colliding
      if (!collision.isColliding(
        newX,
        newY,
        transformable.w,
        transformable.h))
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

          auto& adj = [&] () -> const MappableComponent::Adjacent& {
              switch (collision.getDirection())
              {
                case Direction::left: return mappable.leftAdjacent;
                case Direction::right: return mappable.rightAdjacent;
                case Direction::up: return mappable.upAdjacent;
                case Direction::down: return mappable.downAdjacent;
              }
            }();

          switch (adj.type)
          {
            case MappableComponent::Adjacent::Type::wall:
            {
              touchedWall = true;

              break;
            }

            case MappableComponent::Adjacent::Type::wrap:
            {
              switch (collision.getDirection())
              {
                case Direction::left:
                {
                  newX = GAME_WIDTH + WALL_GAP - transformable.w;

                  break;
                }

                case Direction::right:
                {
                  newX = -WALL_GAP;

                  break;
                }

                case Direction::up:
                {
                  newY = MAP_HEIGHT * TILE_HEIGHT + WALL_GAP - transformable.h;

                  break;
                }

                case Direction::down:
                {
                  newY = -WALL_GAP;

                  break;
                }
              }
            }

            case MappableComponent::Adjacent::Type::warp:
            {
              if (game_.getEntityManager().
                hasComponent<PlayableComponent>(entity))
              {
                adjacentlyWarping = true;
                adjWarpDir = collision.getDirection();
                adjWarpMapId = adj.mapId;
              }

              break;
            }

            case MappableComponent::Adjacent::Type::reverse:
            {
              // TODO: not yet implemented.

              break;
            }
          }

          break;
        }

        case Collision::Type::danger:
        {
          if (game_.getEntityManager().
            hasComponent<PlayableComponent>(entity))
          {
            game_.getSystemManager().getSystem<PlayingSystem>().die(entity);

            adjacentlyWarping = false;
          }

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
            ponderable.velX = 0.0;

            break;
          }

          case Direction::right:
          {
            newX = collision.getAxis() - transformable.w;
            ponderable.velX = 0.0;

            break;
          }

          case Direction::up:
          {
            newY = collision.getAxis();
            ponderable.velY = 0.0;

            break;
          }

          case Direction::down:
          {
            newY = collision.getAxis() - transformable.h;
            ponderable.velY = 0.0;
            ponderable.grounded = true;

            break;
          }
        }
      }
    }

    // Move
    transformable.x = newX;
    transformable.y = newY;

    // Perform cleanup for orientable entites
    if (game_.getEntityManager().hasComponent<OrientableComponent>(entity))
    {
      auto& orientable = game_.getEntityManager().
        getComponent<OrientableComponent>(entity);

      // Handle changes in groundedness
      if (ponderable.grounded != oldGrounded)
      {
        if (ponderable.grounded)
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

    // Move to an adjacent map, if necessary
    if (adjacentlyWarping)
    {
      double warpX = newX;
      double warpY = newY;

      switch (adjWarpDir)
      {
        case Direction::left:
        {
          warpX = GAME_WIDTH + WALL_GAP - transformable.w;

          break;
        }

        case Direction::right:
        {
          warpX = -WALL_GAP;

          break;
        }

        case Direction::up:
        {
          warpY = MAP_HEIGHT * TILE_HEIGHT - transformable.h;

          break;
        }

        case Direction::down:
        {
          warpY = -WALL_GAP;

          break;
        }
      }

      game_.getSystemManager().getSystem<PlayingSystem>().
        changeMap(
          entity,
          adjWarpMapId,
          warpX,
          warpY);
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
    ponderable.accelY = NORMAL_GRAVITY;
  }
}

void PonderingSystem::initPrototype(id_type prototype)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(prototype);

  ponderable.velX = 0.0;
  ponderable.velY = 0.0;
  ponderable.accelX = 0.0;
  ponderable.accelY = 0.0;
  ponderable.grounded = false;
  ponderable.frozen = false;
  ponderable.collidable = true;
}
