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

  // Move
  double newX = transformable.x;
  double newY = transformable.y;

  if (!ponderable.frozen)
  {
    if (ponderable.ferried)
    {
      auto& ferryTrans = game_.getEntityManager().
        getComponent<TransformableComponent>(ponderable.ferry);

      newX = ferryTrans.x + ponderable.relX;
      newY = ferryTrans.y + ponderable.relY;
    }

    newX += ponderable.velX * dt;
    newY += ponderable.velY * dt;
  }

  CollisionResult result =
    moveBody(
      entity,
      newX,
      newY);

  // Perform cleanup for orientable entites
  bool groundedChanged = (ponderable.grounded != result.grounded);
  ponderable.grounded = result.grounded;

  if (game_.getEntityManager().hasComponent<OrientableComponent>(entity))
  {
    auto& orientable = game_.getEntityManager().
      getComponent<OrientableComponent>(entity);

    // Handle changes in groundedness
    if (groundedChanged)
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
      groundedChanged)
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

CollisionResult PonderingSystem::moveBody(
  id_type entity,
  double x,
  double y)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  const double oldX = transformable.x;
  const double oldY = transformable.y;
  const double oldRight = oldX + transformable.w;
  const double oldBottom = oldY + transformable.h;

  CollisionResult result;

  if (ponderable.collidable)
  {
    result = detectCollisions(entity, x, y);
  } else {
    result.newX = x;
    result.newY = y;
  }

  // Move
  if (!ponderable.frozen)
  {
    transformable.x = result.newX;
    transformable.y = result.newY;
  }

  return result;
}

namespace CollisionParams {

  template <typename HorizVert>
  class Desc : public HorizVert {
  public:

    inline static bool AtLeastInAxisSweep(
      double boundaryAxis,
      double entityAxis)
    {
      return (boundaryAxis >= entityAxis);
    }

    inline static bool IsPastAxis(
      double colliderAxis,
      double entityAxis)
    {
      return (colliderAxis > entityAxis);
    }

    inline static double OldAxis(const TransformableComponent& transformable)
    {
      return HorizVert::AxisOldLower(transformable);
    }

    inline static double NewAxis(
      const CollisionResult& result,
      const TransformableComponent&)
    {
      return HorizVert::AxisNewLower(result);
    }

    inline static double ObjectAxis(const TransformableComponent& transformable)
    {
      return HorizVert::AxisOldUpper(transformable);
    }

    inline static bool Closer(double left, double right)
    {
      return right < left;
    }
  };

  template <typename HorizVert>
  class Asc : public HorizVert {
  public:

    inline static bool AtLeastInAxisSweep(
      double boundaryAxis,
      double entityAxis)
    {
      return (boundaryAxis <= entityAxis);
    }

    inline static bool IsPastAxis(
      double colliderAxis,
      double entityAxis)
    {
      return (colliderAxis < entityAxis);
    }

    inline static double OldAxis(const TransformableComponent& transformable)
    {
      return HorizVert::AxisOldUpper(transformable);
    }

    inline static double NewAxis(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return HorizVert::AxisNewUpper(result, transformable);
    }

    inline static double ObjectAxis(const TransformableComponent& transformable)
    {
      return HorizVert::AxisOldLower(transformable);
    }

    inline static bool Closer(double left, double right)
    {
      return left < right;
    }
  };

  class Horizontal {
  public:

    inline static double AxisOldLower(
      const TransformableComponent& transformable)
    {
      return transformable.x;
    }

    inline static double AxisOldUpper(
      const TransformableComponent& transformable)
    {
      return transformable.x + transformable.w;
    }

    inline static double AxisNewLower(const CollisionResult& result)
    {
      return result.newX;
    }

    inline static double AxisNewUpper(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return result.newX + transformable.w;
    }

    inline static double NonAxisOldLower(
      const TransformableComponent& transformable)
    {
      return transformable.y;
    }

    inline static double NonAxisOldUpper(
      const TransformableComponent& transformable)
    {
      return transformable.y + transformable.h;
    }

    inline static double NonAxisNewLower(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return result.newY;
    }

    inline static double NonAxisNewUpper(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return result.newY + transformable.h;
    }
  };

  class Vertical {
  public:

    inline static double AxisOldLower(
      const TransformableComponent& transformable)
    {
      return transformable.y;
    }

    inline static double AxisOldUpper(
      const TransformableComponent& transformable)
    {
      return transformable.y + transformable.h;
    }

    inline static double AxisNewLower(const CollisionResult& result)
    {
      return result.newY;
    }

    inline static double AxisNewUpper(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return result.newY + transformable.h;
    }

    inline static double NonAxisOldLower(
      const TransformableComponent& transformable)
    {
      return transformable.x;
    }

    inline static double NonAxisOldUpper(
      const TransformableComponent& transformable)
    {
      return transformable.x + transformable.w;
    }

    inline static double NonAxisNewLower(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return result.newX;
    }

    inline static double NonAxisNewUpper(
      const CollisionResult& result,
      const TransformableComponent& transformable)
    {
      return result.newX + transformable.w;
    }
  };

  template <Direction dir, typename AscDesc>
  class DetectCollisions : public AscDesc {
  public:

    static const Direction Dir = dir;
  };

  class Left : public DetectCollisions<Direction::left, Desc<Horizontal>> {
  public:

    inline static const MappableComponent::desc_boundaries_type& MapBoundaries(
      const MappableComponent& mappable)
    {
      return mappable.leftBoundaries;
    }
  };

  class Right : public DetectCollisions<Direction::right, Asc<Horizontal>> {
  public:

    inline static const MappableComponent::asc_boundaries_type& MapBoundaries(
      const MappableComponent& mappable)
    {
      return mappable.rightBoundaries;
    }
  };

  class Up : public DetectCollisions<Direction::up, Desc<Vertical>> {
  public:

    inline static const MappableComponent::desc_boundaries_type& MapBoundaries(
      const MappableComponent& mappable)
    {
      return mappable.upBoundaries;
    }
  };

  class Down : public DetectCollisions<Direction::down, Asc<Vertical>> {
  public:

    inline static const MappableComponent::asc_boundaries_type& MapBoundaries(
      const MappableComponent& mappable)
    {
      return mappable.downBoundaries;
    }
  };
};

CollisionResult PonderingSystem::detectCollisions(
  id_type entity,
  double x,
  double y)
{
  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  CollisionResult result;
  result.newX = x;
  result.newY = transformable.y;

  // Find horizontal collisions.
  if (result.newX < transformable.x)
  {
    detectCollisionsInDirection<CollisionParams::Left>(entity, result);
  } else if (result.newX > transformable.x)
  {
    detectCollisionsInDirection<CollisionParams::Right>(entity, result);
  }

  // Find vertical collisions
  if (!result.stopProcessing)
  {
    result.newY = y;
    result.touchedWall = false;

    if (result.newY < transformable.y)
    {
      detectCollisionsInDirection<CollisionParams::Up>(entity, result);
    } else if (result.newY > transformable.y)
    {
      detectCollisionsInDirection<CollisionParams::Down>(entity, result);
    }
  }

  return result;
}

template <typename Param>
void PonderingSystem::detectCollisionsInDirection(
  id_type entity,
  CollisionResult& result)
{
  // Get map data.
  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(
      game_.getSystemManager().getSystem<RealizingSystem>().getSingleton());

  id_type mapEntity = realizable.activeMap;

  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(mapEntity);

  // Get old location.
  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  bool boundaryCollision = false;

  auto boundaries = Param::MapBoundaries(mappable);
  auto it = boundaries.lower_bound(Param::OldAxis(transformable));

  // Find the axis distance of the closest environmental boundary.
  for (;
      (it != std::end(boundaries)) &&
        Param::AtLeastInAxisSweep(
          it->first,
          Param::NewAxis(result, transformable));
      it++)
  {
    // Check that the boundary is in range for the other axis.
    if ((Param::NonAxisNewUpper(result, transformable) > it->second.lower) &&
        (Param::NonAxisNewLower(result, transformable) < it->second.upper))
    {
      // We have a collision!
      boundaryCollision = true;

      break;
    }
  }

  // Find a list of potential colliders, sorted so that the closest is
  // first.
  std::vector<id_type> colliders;

  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    PonderableComponent,
    TransformableComponent>();

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
    if (Param::IsPastAxis(
          Param::ObjectAxis(colliderTrans),
          Param::NewAxis(result, transformable)) &&
        // that it wasn't already colliding,
        !Param::IsPastAxis(
          Param::ObjectAxis(colliderTrans),
          Param::OldAxis(transformable)) &&
        // that the position on the non-axis is in range,
        (Param::NonAxisOldUpper(colliderTrans) >
          Param::NonAxisNewLower(result, transformable)) &&
        (Param::NonAxisOldLower(colliderTrans) <
          Param::NonAxisNewUpper(result, transformable)) &&
        // and that the collider is not farther away than the environmental
        // boundary.
        (!boundaryCollision ||
          Param::AtLeastInAxisSweep(
            Param::ObjectAxis(colliderTrans),
            it->first)))
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

      return Param::Closer(
        Param::ObjectAxis(leftTrans),
        Param::ObjectAxis(rightTrans));
    });

  for (id_type collider : colliders)
  {
    auto& colliderTrans = game_.getEntityManager().
      getComponent<TransformableComponent>(collider);

    // Check if the entity would still move into the potential collider.
    if (!Param::IsPastAxis(
          Param::ObjectAxis(colliderTrans),
          Param::NewAxis(result, transformable)))
    {
      break;
    }

    auto& colliderPonder = game_.getEntityManager().
      getComponent<PonderableComponent>(collider);

    processCollision(
      entity,
      collider,
      Param::Dir,
      colliderPonder.colliderType,
      Param::ObjectAxis(colliderTrans),
      Param::NonAxisOldLower(colliderTrans),
      Param::NonAxisOldUpper(colliderTrans),
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
        (it != std::end(boundaries)) &&
          (it->first == boundaryAxis);
        it++)
    {
      if ((Param::NonAxisNewUpper(result, transformable) > it->second.lower) &&
          (Param::NonAxisNewLower(result, transformable) < it->second.upper))
      {
        processCollision(
          entity,
          mapEntity,
          Param::Dir,
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
        result.grounded = true;

        break;
      }
    }
  }
}
