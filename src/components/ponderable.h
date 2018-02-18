#ifndef TANGIBLE_H_746DB3EE
#define TANGIBLE_H_746DB3EE

#include "component.h"

class PonderableComponent : public Component {
public:

  enum class Type {
    vacuumed,
    freefalling
  };

  PonderableComponent(Type type) : type_(type)
  {
  }

  inline Type getType() const
  {
    return type_;
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

private:

  double velX_ = 0.0;
  double velY_ = 0.0;
  double accelX_ = 0.0;
  double accelY_ = 0.0;
  Type type_ = Type::vacuumed;
  bool grounded_ = false;
  bool frozen_ = false;
  bool collidable_ = true;
};

#endif /* end of include guard: TANGIBLE_H_746DB3EE */
