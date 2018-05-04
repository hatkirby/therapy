#ifndef TANGIBLE_H_746DB3EE
#define TANGIBLE_H_746DB3EE

#include <set>
#include "component.h"
#include "entity_manager.h"

class PonderableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * List of different types of physical bodies.
   *
   * vacuumed     - Default.
   * freefalling  - The body will be treated as if there were a downward force
   *                of gravity being exerted onto it. The body will also exhibit
   *                terminal velocity (that is, its downward velocity will be
   *                capped at a constant value).
   */
  enum class Type {
    vacuumed,
    freefalling
  };

  /**
   * List of different types of collidable surfaces.
   */
  enum class Collision {
    wall,
    platform,
    adjacency,
    warp,
    danger
  };

  /**
   * Constructor for initializing the body type, which is a constant.
   */
  PonderableComponent(Type type) : type(type)
  {
  }

  /**
   * The velocity of the body.
   */
  double velX = 0.0;
  double velY = 0.0;

  /**
   * The acceleration of the body.
   */
  double accelX = 0.0;
  double accelY = 0.0;

  /**
   * The type of physical body that the entity is meant to assume. The body will
   * be acted upon differently based on this. See the enumeration above for more
   * details.
   *
   * @managed_by PonderingSystem
   */
  const Type type;

  /**
   * Whether or not a freefalling body is in contact with the ground.
   *
   * @managed_by PonderingSystem
   */
  bool grounded = false;

  /**
   * Whether or not a freefalling body is being ferried by another body.
   *
   * @managed_by PonderingSystem
   */
  bool ferried = false;

  /**
   * The entity that is ferrying this body, if there is one.
   *
   * @managed_by PonderingSystem
   */
  id_type ferry;

  /**
   * The location of the body relative to the location of its ferry.
   *
   * @managed_by PonderingSystem
   */
  double relX;
  double relY;

  /**
   * The bodies that are being ferried by this body.
   *
   * @managed_by PonderingSystem
   */
  std::set<id_type> passengers;

  /**
   * If enabled, this will prevent the body from moving and accelerating. The
   * velocity and position of the body can still be affected by sources external
   * to the PonderingSystem. Enabling this will cause applicable bodies to
   * become ungrounded and unferried.
   */
  bool frozen = false;

  /**
   * If disabled, collision detection for this body will not be performed and
   * other bodies will ignore it. Disabling this will cause applicable bodies to
   * become ungrounded and unferried.
   */
  bool collidable = true;

  /**
   * The effect that colliding with this body has.
   */
  Collision colliderType = Collision::wall;

  /**
   * If this flag is disabled, the entity will be ignored by the pondering
   * system.
   *
   * @managed_by RealizingSystem
   */
  bool active = false;
};

#endif /* end of include guard: TANGIBLE_H_746DB3EE */
