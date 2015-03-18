#ifndef MAP_COLLISION_H
#define MAP_COLLISION_H

#include "entity.h"
#include "map.h"
#include <list>

class Game;

class MapCollisionComponent : public Component {
  public:
    MapCollisionComponent(const Map& map);
    void detectCollision(Game& game, Entity& entity, Entity& collider, std::pair<double, double> old_position);
    
  private:
    enum class Direction {
      up, left, down, right
    };

    struct Collision {
      enum class Type {
        wall,
        wrap,
        teleport,
        reverse,
        platform,
        danger
      };
      
      double axis;
      double lower;
      double upper;
      Type type;
    };
    
    void addCollision(double axis, double lower, double upper, Direction dir, Collision::Type type);
    void processCollision(Game& game, Entity& collider, Collision collision, Direction dir, std::pair<double, double> old_position);
    Collision::Type collisionFromMoveType(Map::MoveType type);
    
    std::list<Collision> left_collisions;
    std::list<Collision> right_collisions;
    std::list<Collision> up_collisions;
    std::list<Collision> down_collisions;
    const Map& map;
};

#endif
