#ifndef ORIENTABLE_H_EDB6C4A1
#define ORIENTABLE_H_EDB6C4A1

#include "component.h"

class OrientableComponent : public Component {
public:

  inline bool isFacingRight() const
  {
    return facingRight_;
  }

  inline void setFacingRight(bool v)
  {
    facingRight_ = v;
  }

private:

  bool facingRight_ = false;
};

#endif /* end of include guard: ORIENTABLE_H_EDB6C4A1 */
