#ifndef DROPPABLE_H_5DB254EF
#define DROPPABLE_H_5DB254EF

#include "component.h"

class DroppableComponent : public Component {
  public:
    void setDroppable(bool can);
    bool isDroppable() const;
    
  private:
    bool droppable = false;
};

#endif /* end of include guard: DROPPABLE_H_5DB254EF */
