#ifndef DROPPABLE_H_5DB254EF
#define DROPPABLE_H_5DB254EF

#include "component.h"

class DroppableComponent : public Component {
public:

  inline bool isDroppable() const
  {
    return droppable_;
  }

  inline void setDroppable(bool can)
  {
    droppable_ = can;
  }

private:

  bool droppable_ = false;
};

#endif /* end of include guard: DROPPABLE_H_5DB254EF */
