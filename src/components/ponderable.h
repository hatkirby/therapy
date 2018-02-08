#ifndef TANGIBLE_H_746DB3EE
#define TANGIBLE_H_746DB3EE

#include "component.h"

class PonderableComponent : public Component {
public:

  enum class state {
    grounded,
    jumping,
    falling,
    dropping
  };

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

  inline state getState() const
  {
    return state_;
  }

  inline void setState(state arg)
  {
    state_ = arg;
  }

private:

  double velX_ = 0.0;
  double velY_ = 0.0;
  double accelX_ = 0.0;
  double accelY_ = 0.0;
  state state_ = state::grounded;
};

#endif /* end of include guard: TANGIBLE_H_746DB3EE */
