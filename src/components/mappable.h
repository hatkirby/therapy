#ifndef MAPPABLE_H_0B0316FB
#define MAPPABLE_H_0B0316FB

#include <map>
#include <string>
#include <vector>
#include <list>
#include "component.h"
#include "renderer/texture.h"
#include "collision.h"
#include "entity_manager.h"

class MappableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * Helper type that stores information about map adjacency.
   */
  class Adjacent {
  public:

    enum class Type {
      wall,
      wrap,
      warp,
      reverse
    };

    Adjacent(
      Type type = Type::wall,
      size_t mapId = 0) :
        type(type),
        mapId(mapId)
    {
    }

    Type type;
    size_t mapId;
  };

  /**
   * Helper type that stores information about collision boundaries.
   */
  class Boundary {
  public:

    using Type = Collision::Type;

    Boundary(
      double axis,
      double lower,
      double upper,
      Type type) :
        axis(axis),
        lower(lower),
        upper(upper),
        type(type)
    {
    }

    double axis;
    double lower;
    double upper;
    Type type;
  };

  /**
   * Helper types for efficient storage and lookup of collision boundaries.
   */
  using asc_boundaries_type =
    std::multimap<
      double,
      const Boundary,
      std::less<double>>;

  using desc_boundaries_type =
    std::multimap<
      double,
      const Boundary,
      std::greater<double>>;

  /**
   * Constructor for initializing the tileset and font attributes, as they are
   * not default constructible.
   */
  MappableComponent(
    Texture tileset,
    Texture font) :
      tileset(std::move(tileset)),
      font(std::move(font))
  {
  }

  /**
   * The ID of the map in the world definition that this entity represents.
   *
   * @managed_by RealizingSystem
   */
  size_t mapId;

  /**
   * The title of the map, which is displayed at the bottom of the screen.
   */
  std::string title;

  /**
   * The map data.
   *
   * @managed_by RealizingSystem
   */
  std::vector<int> tiles;

  /**
   * These objects describe the behavior of the four edges of the map.
   *
   * @managed_by RealizingSystem
   */
  Adjacent leftAdjacent;
  Adjacent rightAdjacent;
  Adjacent upAdjacent;
  Adjacent downAdjacent;

  /**
   * Collision boundaries, for detecting when a ponderable entity is colliding
   * with the environment.
   *
   * @managed_by MappingSystem
   */
  desc_boundaries_type leftBoundaries;
  asc_boundaries_type rightBoundaries;
  desc_boundaries_type upBoundaries;
  asc_boundaries_type downBoundaries;

  /**
   * The list of entities representing the objects owned by the map.
   *
   * @managed_by RealizingSystem
   */
  std::list<id_type> objects;

  /**
   * The tilesets for the map and the map name.
   *
   * TODO: These probably do not belong here.
   */
  Texture tileset;
  Texture font;
};

#endif /* end of include guard: MAPPABLE_H_0B0316FB */
