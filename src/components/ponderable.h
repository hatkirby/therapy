#ifndef TANGIBLE_H_746DB3EE
#define TANGIBLE_H_746DB3EE

#include "component.h"
#include <functional>
#include <array>

class Game;

class PonderableComponent : public Component {
public:

  enum class BodyType {
    vacuumed,
    freefalling
  };

  enum class ColliderType {
    player,
    event
  };

  static const size_t COLLIDER_TYPES = 2;

  PonderableComponent(
    BodyType bodyType,
    ColliderType colliderType) :
      bodyType_(bodyType),
      colliderType_(colliderType)
  {
  }

  using event_callback_type = std::function<void(Game& game)>;

  inline BodyType getBodyType() const
  {
    return bodyType_;
  }

  inline ColliderType getColliderType() const
  {
    return colliderType_;
  }

  inline double getVelocityX() const
  {
    return velX_;
  }

  inline void setVelocityX(double v)
  {
    velX_ = v;
  }

  inline double getVelocityY() const
  {
    return velY_;
  }

  inline void setVelocityY(double v)
  {
    velY_ = v;
  }

  inline double getAccelX() const
  {
    return accelX_;
  }

  inline void setAccelX(double v)
  {
    accelX_ = v;
  }

  inline double getAccelY() const
  {
    return accelY_;
  }

  inline void setAccelY(double v)
  {
    accelY_ = v;
  }

  inline bool isGrounded() const
  {
    return grounded_;
  }

  inline void setGrounded(bool v)
  {
    grounded_ = v;
  }

  inline bool isFrozen() const
  {
    return frozen_;
  }

  inline void setFrozen(bool v)
  {
    frozen_ = v;
  }

  inline bool isCollidable() const
  {
    return collidable_;
  }

  inline void setCollidable(bool v)
  {
    collidable_ = v;
  }

  inline const event_callback_type& getEventCallback(ColliderType v) const
  {
    return eventCallbacks_[static_cast<size_t>(v)];
  }

  inline void setEventCallback(ColliderType v, event_callback_type callback)
  {
    eventCallbacks_[static_cast<size_t>(v)] = std::move(callback);
  }

private:

  double velX_ = 0.0;
  double velY_ = 0.0;
  double accelX_ = 0.0;
  double accelY_ = 0.0;
  BodyType bodyType_;
  ColliderType colliderType_;
  bool grounded_ = false;
  bool frozen_ = false;
  bool collidable_ = true;
  std::array<event_callback_type, COLLIDER_TYPES> eventCallbacks_;
};

#endif /* end of include guard: TANGIBLE_H_746DB3EE */
