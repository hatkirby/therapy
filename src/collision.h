#ifndef COLLISION_H_53D84877
#define COLLISION_H_53D84877

#include "entity_manager.h"
#include "direction.h"

class Collision {
public:

  using id_type = EntityManager::id_type;

  // Types are defined in descending priority order
  enum class Type {
    wall,
    platform,
    adjacency,
    warp,
    danger
  };

  Collision(
    id_type collider,
    Direction dir,
    Type type,
    int axis,
    double lower,
    double upper) :
      collider_(collider),
      dir_(dir),
      type_(type),
      axis_(axis),
      lower_(lower),
      upper_(upper)
  {
  }

  inline id_type getCollider() const
  {
    return collider_;
  }

  inline Direction getDirection() const
  {
    return dir_;
  }

  inline Type getType() const
  {
    return type_;
  }

  inline int getAxis() const
  {
    return axis_;
  }

  inline double getLower() const
  {
    return lower_;
  }

  inline double getUpper() const
  {
    return upper_;
  }

  bool operator<(const Collision& other) const;

  bool isColliding(double x, double y, int w, int h) const;

private:

  id_type collider_;
  Direction dir_;
  Type type_;
  int axis_;
  double lower_;
  double upper_;
};

#endif /* end of include guard: COLLISION_H_53D84877 */
