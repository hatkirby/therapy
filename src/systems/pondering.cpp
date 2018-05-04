#include "pondering.h"
#include <queue>
#include <algorithm>
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

void PonderingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    PonderableComponent,
    TransformableComponent>();

  for (id_type entity : entities)
  {
    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    // We will recursively process ferried bodies after their ferries have been
    // processed, so hold off on processing ferried bodies at the top level.
    if (ponderable.ferried)
    {
      continue;
    }

    tickBody(
      entity,
      dt,
      entities);
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
  ponderable.ferried = false;
  ponderable.passengers.clear();
}

void PonderingSystem::unferry(id_type entity)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  if (ponderable.ferried)
  {
    ponderable.ferried = false;

    auto& ferryPonder = game_.getEntityManager().
      getComponent<PonderableComponent>(ponderable.ferry);

    ferryPonder.passengers.erase(entity);
  }
}

void PonderingSystem::tickBody(
  id_type entity,
  double dt,
  const std::set<id_type>& entities)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  if (!ponderable.active)
  {
    return;
  }

  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(
      game_.getSystemManager().getSystem<RealizingSystem>().getSingleton());

  id_type mapEntity = realizable.activeMap;

  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(mapEntity);

  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  // Accelerate
  if (!ponderable.frozen)
  {
    ponderable.velX += ponderable.accelX * dt;
    ponderable.velY += ponderable.accelY * dt;

    if ((ponderable.type == PonderableComponent::Type::freefalling)
      && (ponderable.velY > TERMINAL_VELOCITY))
    {
      ponderable.velY = TERMINAL_VELOCITY;
    }
  }

  const double oldX = transformable.x;
  const double oldY = transformable.y;
  const double oldRight = oldX + transformable.w;
  const double oldBottom = oldY + transformable.h;

  CollisionResult result;
  result.newX = transformable.x;
  result.newY = transformable.y;

  if (!ponderable.frozen)
  {
    if (ponderable.ferried)
    {
      auto& ferryTrans = game_.getEntityManager().
        getComponent<TransformableComponent>(ponderable.ferry);

      result.newX = ferryTrans.x + ponderable.relX;
      result.newY = ferryTrans.y + ponderable.relY;
    }

    result.newX += ponderable.velX * dt;
    result.newY += ponderable.velY * dt;
  }

  bool oldGrounded = ponderable.grounded;
  ponderable.grounded = false;

  if (ponderable.collidable)
  {
    // Find horizontal collisions.
    if (result.newX < oldX)
    {
      bool boundaryCollision = false;
      auto it = mappable.leftBoundaries.lower_bound(oldX);

      // Find the axis distance of the closest environmental boundary.
      for (;
          (it != std::end(mappable.leftBoundaries)) &&
            (it->first >= result.newX);
          it++)
      {
        // Check that the boundary is in range for the other axis.
        if ((oldBottom > it->second.lower) && (oldY < it->second.upper))
        {
          // We have a collision!
          boundaryCollision = true;

          break;
        }
      }

      // Find a list of potential colliders, sorted so that the closest is
      // first.
      std::vector<id_type> colliders;

      for (id_type collider : entities)
      {
        // Can't collide with self.
        if (collider == entity)
        {
          continue;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        // Only check objects that are active and collidable.
        if (!colliderPonder.active || !colliderPonder.collidable)
        {
          continue;
        }

        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would move into the potential collider,
        if ((colliderTrans.x + colliderTrans.w > result.newX) &&
            // that it wasn't already colliding,
            (colliderTrans.x + colliderTrans.w <= oldX) &&
            // that the position on the other axis is in range,
            (colliderTrans.y + colliderTrans.h > oldY) &&
            (colliderTrans.y < oldBottom) &&
            // and that the collider is not farther away than the environmental
            // boundary.
            (!boundaryCollision ||
              (colliderTrans.x + colliderTrans.w >= it->first)))
        {
          colliders.push_back(collider);
        }
      }

      std::sort(
        std::begin(colliders),
        std::end(colliders),
        [&] (id_type left, id_type right) {
          auto& leftTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(left);

          auto& rightTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(right);

          return (rightTrans.x < leftTrans.x);
        });

      for (id_type collider : colliders)
      {
        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would still move into the potential collider.
        if (colliderTrans.x + colliderTrans.w <= result.newX)
        {
          break;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        processCollision(
          entity,
          collider,
          Direction::left,
          colliderPonder.colliderType,
          colliderTrans.x + colliderTrans.w,
          colliderTrans.y,
          colliderTrans.y + colliderTrans.h,
          result);

        if (result.stopProcessing)
        {
          break;
        }
      }

      // If movement hasn't been stopped by an intermediary object, and
      // collision checking hasn't been stopped, process the environmental
      // boundaries closest to the entity.
      if (!result.stopProcessing && !result.touchedWall && boundaryCollision)
      {
        double boundaryAxis = it->first;

        for (;
            (it != std::end(mappable.leftBoundaries)) &&
              (it->first == boundaryAxis);
            it++)
        {
          if ((oldBottom > it->second.lower) && (oldY < it->second.upper))
          {
            processCollision(
              entity,
              mapEntity,
              Direction::left,
              it->second.type,
              it->first,
              it->second.lower,
              it->second.upper,
              result);

            if (result.stopProcessing)
            {
              break;
            }
          }
        }
      }
    } else if (result.newX > oldX)
    {
      bool boundaryCollision = false;
      auto it = mappable.rightBoundaries.lower_bound(oldRight);

      // Find the axis distance of the closest environmental boundary.
      for (;
          (it != std::end(mappable.rightBoundaries))
            && (it->first <= (result.newX + transformable.w));
          it++)
      {
        // Check that the boundary is in range for the other axis.
        if ((oldBottom > it->second.lower) && (oldY < it->second.upper))
        {
          // We have a collision!
          boundaryCollision = true;

          break;
        }
      }

      // Find a list of potential colliders, sorted so that the closest is
      // first.
      std::vector<id_type> colliders;

      for (id_type collider : entities)
      {
        // Can't collide with self.
        if (collider == entity)
        {
          continue;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        // Only check objects that are active and collidable.
        if (!colliderPonder.active || !colliderPonder.collidable)
        {
          continue;
        }

        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would move into the potential collider,
        if ((colliderTrans.x < result.newX + transformable.w) &&
            // that it wasn't already colliding,
            (colliderTrans.x >= oldRight) &&
            // that the position on the other axis is in range,
            (colliderTrans.y + colliderTrans.h > oldY) &&
            (colliderTrans.y < oldBottom) &&
            // and that the collider is not farther away than the environmental
            // boundary.
            (!boundaryCollision || (colliderTrans.x <= it->first)))
        {
          colliders.push_back(collider);
        }
      }

      std::sort(
        std::begin(colliders),
        std::end(colliders),
        [&] (id_type left, id_type right) {
          auto& leftTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(left);

          auto& rightTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(right);

          return (leftTrans.x < rightTrans.x);
        });

      for (id_type collider : colliders)
      {
        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would still move into the potential collider.
        if (colliderTrans.x >= result.newX + transformable.w)
        {
          break;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        processCollision(
          entity,
          collider,
          Direction::right,
          colliderPonder.colliderType,
          colliderTrans.x,
          colliderTrans.y,
          colliderTrans.y + colliderTrans.h,
          result);

        if (result.stopProcessing)
        {
          break;
        }
      }

      // If movement hasn't been stopped by an intermediary object, and
      // collision checking hasn't been stopped, process the environmental
      // boundaries closest to the entity.
      if (!result.stopProcessing && !result.touchedWall && boundaryCollision)
      {
        double boundaryAxis = it->first;

        for (;
            (it != std::end(mappable.rightBoundaries)) &&
              (it->first == boundaryAxis);
            it++)
        {
          if ((oldBottom > it->second.lower) && (oldY < it->second.upper))
          {
            processCollision(
              entity,
              mapEntity,
              Direction::right,
              it->second.type,
              it->first,
              it->second.lower,
              it->second.upper,
              result);

            if (result.stopProcessing)
            {
              break;
            }
          }
        }
      }
    }
  }

  // Find vertical collisions
  if (ponderable.collidable && !result.stopProcessing)
  {
    result.touchedWall = false;

    if (result.newY < oldY)
    {
      bool boundaryCollision = false;
      auto it = mappable.upBoundaries.lower_bound(oldY);

      // Find the axis distance of the closest environmental boundary.
      for (;
          (it != std::end(mappable.upBoundaries)) &&
            (it->first >= result.newY);
          it++)
      {
        // Check that the boundary is in range for the other axis.
        if ((result.newX + transformable.w > it->second.lower) &&
            (result.newX < it->second.upper))
        {
          // We have a collision!
          boundaryCollision = true;

          break;
        }
      }

      // Find a list of potential colliders, sorted so that the closest is
      // first.
      std::vector<id_type> colliders;

      for (id_type collider : entities)
      {
        // Can't collide with self.
        if (collider == entity)
        {
          continue;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        // Only check objects that are active and collidable.
        if (!colliderPonder.active || !colliderPonder.collidable)
        {
          continue;
        }

        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would move into the potential collider,
        if ((colliderTrans.y + colliderTrans.h > result.newY) &&
            // that it wasn't already colliding,
            (colliderTrans.y + colliderTrans.h <= oldY) &&
            // that the position on the other axis is in range,
            (colliderTrans.x + colliderTrans.w > result.newX) &&
            (colliderTrans.x < result.newX + transformable.w) &&
            // and that the collider is not farther away than the environmental
            // boundary.
            (!boundaryCollision ||
              (colliderTrans.y + colliderTrans.h >= it->first)))
        {
          colliders.push_back(collider);
        }
      }

      std::sort(
        std::begin(colliders),
        std::end(colliders),
        [&] (id_type left, id_type right) {
          auto& leftTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(left);

          auto& rightTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(right);

          return (rightTrans.y < leftTrans.y);
        });

      for (id_type collider : colliders)
      {
        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would still move into the potential collider.
        if (colliderTrans.y + colliderTrans.h <= result.newY)
        {
          break;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        processCollision(
          entity,
          collider,
          Direction::up,
          colliderPonder.colliderType,
          colliderTrans.y + colliderTrans.h,
          colliderTrans.x,
          colliderTrans.x + colliderTrans.w,
          result);

        if (result.stopProcessing)
        {
          break;
        }
      }

      // If movement hasn't been stopped by an intermediary object, and
      // collision checking hasn't been stopped, process the environmental
      // boundaries closest to the entity.
      if (!result.stopProcessing && !result.touchedWall && boundaryCollision)
      {
        double boundaryAxis = it->first;

        for (;
            (it != std::end(mappable.upBoundaries)) &&
              (it->first == boundaryAxis);
            it++)
        {
          if ((result.newX + transformable.w > it->second.lower) &&
              (result.newX < it->second.upper))
          {
            processCollision(
              entity,
              mapEntity,
              Direction::up,
              it->second.type,
              it->first,
              it->second.lower,
              it->second.upper,
              result);

            if (result.stopProcessing)
            {
              break;
            }
          }
        }
      }
    } else if (result.newY > oldY)
    {
      bool boundaryCollision = false;
      auto it = mappable.downBoundaries.lower_bound(oldBottom);

      // Find the axis distance of the closest environmental boundary.
      for (;
          (it != std::end(mappable.downBoundaries))
            && (it->first <= (result.newY + transformable.h));
          it++)
      {
        // Check that the boundary is in range for the other axis.
        if ((result.newX + transformable.w > it->second.lower) &&
            (result.newX < it->second.upper))
        {
          // We have a collision!
          boundaryCollision = true;

          break;
        }
      }

      // Find a list of potential colliders, sorted so that the closest is
      // first.
      std::vector<id_type> colliders;

      for (id_type collider : entities)
      {
        // Can't collide with self.
        if (collider == entity)
        {
          continue;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        // Only check objects that are active and collidable.
        if (!colliderPonder.active || !colliderPonder.collidable)
        {
          continue;
        }

        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would move into the potential collider,
        if ((colliderTrans.y < result.newY + transformable.h) &&
            // that it wasn't already colliding,
            (colliderTrans.y >= oldBottom) &&
            // that the position on the other axis is in range,
            (colliderTrans.x + colliderTrans.w > result.newX) &&
            (colliderTrans.x < result.newX + transformable.w) &&
            // and that the collider is not farther away than the environmental
            // boundary.
            (!boundaryCollision || (colliderTrans.y <= it->first)))
        {
          colliders.push_back(collider);
        }
      }

      std::sort(
        std::begin(colliders),
        std::end(colliders),
        [&] (id_type left, id_type right) {
          auto& leftTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(left);

          auto& rightTrans = game_.getEntityManager().
            getComponent<TransformableComponent>(right);

          return (leftTrans.y < rightTrans.y);
        });

      for (id_type collider : colliders)
      {
        auto& colliderTrans = game_.getEntityManager().
          getComponent<TransformableComponent>(collider);

        // Check if the entity would still move into the potential collider.
        if (colliderTrans.y >= result.newY + transformable.h)
        {
          break;
        }

        auto& colliderPonder = game_.getEntityManager().
          getComponent<PonderableComponent>(collider);

        processCollision(
          entity,
          collider,
          Direction::down,
          colliderPonder.colliderType,
          colliderTrans.y,
          colliderTrans.x,
          colliderTrans.x + colliderTrans.w,
          result);

        if (result.stopProcessing)
        {
          break;
        }
      }

      // If movement hasn't been stopped by an intermediary object, and
      // collision checking hasn't been stopped, process the environmental
      // boundaries closest to the entity.
      if (!result.stopProcessing && !result.touchedWall && boundaryCollision)
      {
        double boundaryAxis = it->first;

        for (;
            (it != std::end(mappable.downBoundaries)) &&
              (it->first == boundaryAxis);
            it++)
        {
          if ((result.newX + transformable.w > it->second.lower) &&
              (result.newX < it->second.upper))
          {
            processCollision(
              entity,
              mapEntity,
              Direction::down,
              it->second.type,
              it->first,
              it->second.lower,
              it->second.upper,
              result);

            if (result.stopProcessing)
            {
              break;
            }
          }
        }
      }
    }
  }

  // Move
  if (!ponderable.frozen)
  {
    transformable.x = result.newX;
    transformable.y = result.newY;
  }

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

  // Ferry or unferry as necessary
  if ((ponderable.type == PonderableComponent::Type::freefalling) &&
      (ponderable.grounded != oldGrounded))
  {
    if (ponderable.grounded &&
        game_.getEntityManager().
          hasComponent<PonderableComponent>(result.groundEntity))
    {
      // The body is now being ferried
      auto& ferryPonder = game_.getEntityManager().
        getComponent<PonderableComponent>(result.groundEntity);

      ponderable.ferried = true;
      ponderable.ferry = result.groundEntity;

      ferryPonder.passengers.insert(entity);
    } else if (ponderable.ferried)
    {
      // The body is no longer being ferried
      unferry(entity);
    }
  }

  // Update a ferry passenger's relative position
  if (ponderable.ferried)
  {
    auto& ferryTrans = game_.getEntityManager().
      getComponent<TransformableComponent>(ponderable.ferry);

    ponderable.relX = transformable.x - ferryTrans.x;
    ponderable.relY = transformable.y - ferryTrans.y;
  }

  // Handle ferry passengers
  std::set<id_type> passengers = ponderable.passengers;

  for (id_type passenger : passengers)
  {
    tickBody(
      passenger,
      dt,
      entities);
  }

  // Move to an adjacent map, if necessary
  if (result.adjacentlyWarping)
  {
    double warpX = result.newX;
    double warpY = result.newY;

    switch (result.adjWarpDir)
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
        result.adjWarpMapId,
        warpX,
        warpY);
  }
}

void PonderingSystem::processCollision(
  id_type entity,
  id_type collider,
  Direction dir,
  PonderableComponent::Collision type,
  double axis,
  double lower,
  double upper,
  CollisionResult& result)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  switch (type)
  {
    case PonderableComponent::Collision::wall:
    {
      result.touchedWall = true;

      break;
    }

    case PonderableComponent::Collision::platform:
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
          result.touchedWall = true;
        }
      } else {
        result.touchedWall = true;
      }

      break;
    }

    case PonderableComponent::Collision::adjacency:
    {
      auto& mappable = game_.getEntityManager().
        getComponent<MappableComponent>(collider);

      auto& adj = [&] () -> const MappableComponent::Adjacent& {
          switch (dir)
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
          result.touchedWall = true;

          break;
        }

        case MappableComponent::Adjacent::Type::wrap:
        {
          switch (dir)
          {
            case Direction::left:
            {
              result.newX = GAME_WIDTH + WALL_GAP - transformable.w;

              break;
            }

            case Direction::right:
            {
              result.newX = -WALL_GAP;

              break;
            }

            case Direction::up:
            {
              result.newY =
                MAP_HEIGHT * TILE_HEIGHT + WALL_GAP - transformable.h;

              break;
            }

            case Direction::down:
            {
              result.newY = -WALL_GAP;

              break;
            }
          }
        }

        case MappableComponent::Adjacent::Type::warp:
        {
          if (game_.getEntityManager().
            hasComponent<PlayableComponent>(entity))
          {
            result.adjacentlyWarping = true;
            result.adjWarpDir = dir;
            result.adjWarpMapId = adj.mapId;
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

    case PonderableComponent::Collision::danger:
    {
      if (game_.getEntityManager().
        hasComponent<PlayableComponent>(entity))
      {
        game_.getSystemManager().getSystem<PlayingSystem>().die(entity);

        result.adjacentlyWarping = false;
      }

      result.stopProcessing = true;

      break;
    }

    default:
    {
      // Not yet implemented.

      break;
    }
  }

  if (!result.stopProcessing && result.touchedWall)
  {
    switch (dir)
    {
      case Direction::left:
      {
        result.newX = axis;
        ponderable.velX = 0.0;

        break;
      }

      case Direction::right:
      {
        result.newX = axis - transformable.w;
        ponderable.velX = 0.0;

        break;
      }

      case Direction::up:
      {
        result.newY = axis;
        ponderable.velY = 0.0;

        break;
      }

      case Direction::down:
      {
        result.newY = axis - transformable.h;
        result.groundEntity = collider;
        ponderable.velY = 0.0;
        ponderable.grounded = true;

        break;
      }
    }
  }
}
