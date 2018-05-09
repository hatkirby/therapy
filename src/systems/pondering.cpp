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

    tickBody(entity, dt);
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
    ponderable.accel.y() = NORMAL_GRAVITY;
  }
}

void PonderingSystem::initPrototype(id_type prototype)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(prototype);

  ponderable.vel.x() = 0.0;
  ponderable.vel.y() = 0.0;
  ponderable.accel.x() = 0.0;
  ponderable.accel.y() = 0.0;
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
  double dt)
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
    ponderable.vel += ponderable.accel * dt;

    if ((ponderable.type == PonderableComponent::Type::freefalling)
      && (ponderable.vel.y() > TERMINAL_VELOCITY))
    {
      ponderable.vel.y() = TERMINAL_VELOCITY;
    }
  }

  // Move
  vec2d newPos = transformable.pos;

  if (!ponderable.frozen)
  {
    newPos += ponderable.vel * dt;
  }

  CollisionResult result = moveBody(entity, newPos);

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
      ponderable.ferrySide = Direction::up;

      ferryPonder.passengers.insert(entity);
    } else if (ponderable.ferried)
    {
      // The body is no longer being ferried
      unferry(entity);
    }
  }

  // Handle ferry passengers
  std::set<id_type> passengers = ponderable.passengers;

  for (id_type passenger : passengers)
  {
    tickBody(passenger, dt);
  }
}

PonderingSystem::CollisionResult PonderingSystem::moveBody(
  id_type entity,
  vec2d newPos)
{
  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  CollisionResult result;

  if (ponderable.collidable)
  {
    result = detectCollisions(entity, newPos);
  } else {
    result.pos = newPos;
  }

  if (!ponderable.frozen)
  {
    auto& transformable = game_.getEntityManager().
      getComponent<TransformableComponent>(entity);

    vec2d delta = result.pos - transformable.pos;

    // Move.
    transformable.pos = result.pos;

    // Stop if the entity hit a wall.
    if (result.blockedHoriz)
    {
      ponderable.vel.x() = 0.0;
    }

    if (result.blockedVert)
    {
      ponderable.vel.y() = 0.0;
    }

    // Move ferry passengers by the appropriate amount.
    auto passengers = ponderable.passengers;

    for (id_type passenger : passengers)
    {
      auto& passTrans = game_.getEntityManager().
        getComponent<TransformableComponent>(passenger);

      moveBody(passenger, passTrans.pos + delta);
    }

    // Move to an adjacent map, if necessary
    if (result.adjacentlyWarping)
    {
      vec2d warpPos = result.pos;

      switch (result.adjWarpDir)
      {
        case Direction::left:
        {
          warpPos.x() = GAME_WIDTH + WALL_GAP - transformable.size.w();

          break;
        }

        case Direction::right:
        {
          warpPos.x() = -WALL_GAP;

          break;
        }

        case Direction::up:
        {
          warpPos.y() = MAP_HEIGHT * TILE_HEIGHT - transformable.size.h();

          break;
        }

        case Direction::down:
        {
          warpPos.y() = -WALL_GAP;

          break;
        }
      }

      game_.getSystemManager().getSystem<PlayingSystem>().
        changeMap(
          entity,
          result.adjWarpMapId,
          warpPos);
    }
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

    inline static double EntityAxis(const vec2d& pos, const vec2i& size)
    {
      return HorizVert::AxisLower(pos);
    }

    inline static double ObjectAxis(const vec2d& pos, const vec2i& size)
    {
      return HorizVert::AxisUpper(pos, size);
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

    inline static double EntityAxis(const vec2d& pos, const vec2i& size)
    {
      return HorizVert::AxisUpper(pos, size);
    }

    inline static double ObjectAxis(const vec2d& pos, const vec2i& size)
    {
      return HorizVert::AxisLower(pos);
    }

    inline static bool Closer(double left, double right)
    {
      return left < right;
    }
  };

  template <size_t Axis, size_t NonAxis>
  class HorizVert {
  public:

    inline static double AxisLower(const vec2d& pos)
    {
      return pos.coords[Axis];
    }

    inline static double AxisUpper(const vec2d& pos, const vec2i& size)
    {
      return pos.coords[Axis] + size.coords[Axis];
    }

    inline static double NonAxisLower(const vec2d& pos)
    {
      return pos.coords[NonAxis];
    }

    inline static double NonAxisUpper(const vec2d& pos, const vec2i& size)
    {
      return pos.coords[NonAxis] + size.coords[NonAxis];
    }

  };

  using Horizontal = HorizVert<0, 1>;
  using Vertical = HorizVert<1, 0>;

  template <Direction dir, typename AscDesc>
  class DetectCollisions : public AscDesc {
  public:

    static const Direction Dir = dir;

    inline static double EntityAxis(const vec2d& pos, const vec2i& size)
    {
      return AscDesc::EntityAxis(pos, size);
    }

    inline static double ObjectAxis(const vec2d& pos, const vec2i& size)
    {
      return AscDesc::ObjectAxis(pos, size);
    }

    inline static double EntityAxis(const TransformableComponent& transformable)
    {
      return AscDesc::EntityAxis(transformable.pos, transformable.size);
    }

    inline static double ObjectAxis(const TransformableComponent& transformable)
    {
      return AscDesc::ObjectAxis(transformable.pos, transformable.size);
    }
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

PonderingSystem::CollisionResult PonderingSystem::detectCollisions(
  id_type entity,
  vec2d newPos)
{
  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  CollisionResult result;
  result.pos.x() = newPos.x();
  result.pos.y() = transformable.pos.y();

  // Find horizontal collisions.
  if (result.pos.x() < transformable.pos.x())
  {
    detectCollisionsInDirection<CollisionParams::Left>(entity, result);
  } else if (result.pos.x() > transformable.pos.x())
  {
    detectCollisionsInDirection<CollisionParams::Right>(entity, result);
  }

  // Find vertical collisions
  if (!result.stopProcessing)
  {
    result.pos.y() = newPos.y();
    result.touchedWall = false;

    if (result.pos.y() < transformable.pos.y())
    {
      detectCollisionsInDirection<CollisionParams::Up>(entity, result);
    } else if (result.pos.y() > transformable.pos.y())
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
  auto& transform = game_.getEntityManager().
    getComponent<TransformableComponent>(entity);

  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(entity);

  bool boundaryCollision = false;

  auto boundaries = Param::MapBoundaries(mappable);
  auto it = boundaries.lower_bound(Param::EntityAxis(transform));

  // Find the axis distance of the closest environmental boundary.
  for (;
      (it != std::end(boundaries)) &&
        Param::AtLeastInAxisSweep(
          it->first,
          Param::EntityAxis(result.pos, transform.size));
      it++)
  {
    // Check that the boundary is in range for the other axis.
    if ((Param::NonAxisUpper(result.pos, transform.size) > it->second.lower) &&
        (Param::NonAxisLower(result.pos) < it->second.upper))
    {
      // We have a collision!
      boundaryCollision = true;

      break;
    }
  }

  // Find the results of pretending to move the entity's passengers, if there
  // are any.
  vec2d delta = result.pos - transform.pos;
  std::map<id_type, CollisionResult> passResults;

  for (id_type passenger : ponderable.passengers)
  {
    auto& passPonder = game_.getEntityManager().
      getComponent<PonderableComponent>(passenger);

    if (passPonder.ferrySide == Param::Dir)
    {
      auto& passTrans = game_.getEntityManager().
        getComponent<TransformableComponent>(passenger);

      passResults[passenger] =
        detectCollisions(passenger, passTrans.pos + delta);
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

    // If the collider is a passenger of the entity, pretend that it has already
    // moved.
    auto& colliderTrans = game_.getEntityManager().
      getComponent<TransformableComponent>(collider);

    vec2d colliderPos = colliderTrans.pos;
    vec2i colliderSize = colliderTrans.size;

    if (passResults.count(collider))
    {
      colliderPos = passResults[collider].pos;
    }

    // Check if the entity would move into the potential collider,
    if (Param::IsPastAxis(
          Param::ObjectAxis(colliderPos, colliderSize),
          Param::EntityAxis(result.pos, transform.size)) &&
        // that it wasn't already colliding,
        !Param::IsPastAxis(
          Param::ObjectAxis(colliderPos, colliderSize),
          Param::EntityAxis(transform)) &&
        // that the position on the non-axis is in range,
        (Param::NonAxisUpper(colliderPos, colliderSize) >
          Param::NonAxisLower(result.pos)) &&
        (Param::NonAxisLower(colliderPos) <
          Param::NonAxisUpper(result.pos, transform.size)) &&
        // and that the collider is not farther away than the environmental
        // boundary.
        (!boundaryCollision ||
          Param::AtLeastInAxisSweep(
            Param::ObjectAxis(colliderPos, colliderSize),
            it->first)))
    {
      colliders.push_back(collider);
    }
  }

  // Sort the potential colliders such that the closest to the axis of movement
  // is first. When sorting, treat passengers of the entity as having already
  // moved.
  std::sort(
    std::begin(colliders),
    std::end(colliders),
    [&] (id_type left, id_type right) {
      auto& leftTrans = game_.getEntityManager().
        getComponent<TransformableComponent>(left);

      vec2d leftPos = leftTrans.pos;

      if (passResults.count(left))
      {
        leftPos = passResults[left].pos;
      }

      auto& rightTrans = game_.getEntityManager().
        getComponent<TransformableComponent>(right);

      vec2d rightPos = rightTrans.pos;

      if (passResults.count(right))
      {
        rightPos = passResults[right].pos;
      }

      return Param::Closer(
        Param::ObjectAxis(leftPos, leftTrans.size),
        Param::ObjectAxis(rightPos, rightTrans.size));
    });

  for (id_type collider : colliders)
  {
    auto& colliderTrans = game_.getEntityManager().
      getComponent<TransformableComponent>(collider);

    // If the collider is a passenger of the entity, pretend that it has already
    // moved.
    vec2d colliderPos = colliderTrans.pos;
    vec2i colliderSize = colliderTrans.size;

    if (passResults.count(collider))
    {
      colliderPos = passResults[collider].pos;
    }

    // Check if the entity would still move into the potential collider.
    if (!Param::IsPastAxis(
          Param::ObjectAxis(colliderPos, colliderSize),
          Param::EntityAxis(result.pos, transform.size)))
    {
      break;
    }

    // TODO: Check if the entity is moving into one of its passengers.
    auto& colliderPonder = game_.getEntityManager().
      getComponent<PonderableComponent>(collider);

    processCollision(
      entity,
      collider,
      Param::Dir,
      colliderPonder.colliderType,
      Param::ObjectAxis(colliderPos, colliderSize),
      Param::NonAxisLower(colliderPos),
      Param::NonAxisUpper(colliderPos, colliderSize),
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
      if ((Param::NonAxisLower(result.pos) < it->second.upper) &&
          (Param::NonAxisUpper(result.pos, transform.size) > it->second.lower))
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
              result.pos.x() = GAME_WIDTH + WALL_GAP - transformable.size.w();

              break;
            }

            case Direction::right:
            {
              result.pos.x() = -WALL_GAP;

              break;
            }

            case Direction::up:
            {
              result.pos.y() =
                MAP_HEIGHT * TILE_HEIGHT + WALL_GAP - transformable.pos.h();

              break;
            }

            case Direction::down:
            {
              result.pos.y() = -WALL_GAP;

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
        result.pos.x() = axis;
        result.blockedHoriz = true;

        break;
      }

      case Direction::right:
      {
        result.pos.x() = axis - transformable.size.w();
        result.blockedHoriz = true;

        break;
      }

      case Direction::up:
      {
        result.pos.y() = axis;
        result.blockedVert = true;

        break;
      }

      case Direction::down:
      {
        result.pos.y() = axis - transformable.size.h();
        result.blockedVert = true;
        result.groundEntity = collider;
        result.grounded = true;

        break;
      }
    }
  }
}
