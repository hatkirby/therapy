#ifndef TANGIBLE_H_746DB3EE
#define TANGIBLE_H_746DB3EE

#include "component.h"

class PonderableComponent : public Component {
  public:
    double getVelocityX() const;
    void setVelocityX(double v);
    double getVelocityY() const;
    void setVelocityY(double v);
    double getAccelX() const;
    void setAccelX(double v);
    double getAccelY() const;
    void setAccelY(double v);

  private:
    double velocityX = 0.0;
    double velocityY = 0.0;
    double accelX = 0.0;
    double accelY = 0.0;
};

#endif /* end of include guard: TANGIBLE_H_746DB3EE */
