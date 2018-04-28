#ifndef TANGIBLE_H_746DB3EE
#define TANGIBLE_H_746DB3EE

#include "component.h"

class PonderableComponent : public Component {
public:

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
   * If enabled, this will prevent the body from moving.
   */
  bool frozen = false;

  /**
   * If disabled, collision detection for this body will not be performed and
   * other bodies will ignore it.
   */
  bool collidable = true;

  /**
   * If this flag is disabled, the entity will be ignored by the pondering
   * system.
   *
   * @managed_by RealizingSystem
   */
  bool active = false;
};

#endif /* end of include guard: TANGIBLE_H_746DB3EE */
