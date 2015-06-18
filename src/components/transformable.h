#ifndef LOCATABLE_H_39E526CA
#define LOCATABLE_H_39E526CA

#include "component.h"

class TransformableComponent : public Component {
  public:
    TransformableComponent(double x, double y, int w, int h);
    
    double getX() const;
    double getY() const;
    int getW() const;
    int getH() const;
    
    void setX(double v);
    void setY(double v);
    void setW(int v);
    void setH(int v);
    
  private:
    double x;
    double y;
    int w;
    int h;
};

#endif /* end of include guard: LOCATABLE_H_39E526CA */
