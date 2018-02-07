#ifndef LOCATABLE_H_39E526CA
#define LOCATABLE_H_39E526CA

#include "component.h"

class TransformableComponent : public Component {
public:

  TransformableComponent(
    double x,
    double y,
    int w,
    int h) :
      x_(x),
      y_(y),
      w_(w),
      h_(h)
  {
  }

  inline double getX() const
  {
    return x_;
  }

  inline void setX(double v)
  {
    x_ = v;
  }

  inline double getY() const
  {
    return y_;
  }

  inline void setY(double v)
  {
    y_ = v;
  }

  inline int getW() const
  {
    return w_;
  }

  inline void setW(int v)
  {
    w_ = v;
  }

  inline int getH() const
  {
    return h_;
  }

  inline void setH(int v)
  {
    h_ = v;
  }

private:

  double x_;
  double y_;
  int w_;
  int h_;
};

#endif /* end of include guard: LOCATABLE_H_39E526CA */
