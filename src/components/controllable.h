#ifndef CONTROLLABLE_H_4E0B85B4
#define CONTROLLABLE_H_4E0B85B4

#include "component.h"
#include "renderer/gl.h"

class ControllableComponent : public Component {
public:

  inline int getLeftKey() const
  {
    return leftKey_;
  }

  inline void setLeftKey(int k)
  {
    leftKey_ = k;
  }

  inline int getRightKey() const
  {
    return rightKey_;
  }

  inline void setRightKey(int k)
  {
    rightKey_ = k;
  }

  inline int getJumpKey() const
  {
    return jumpKey_;
  }

  inline void setJumpKey(int k)
  {
    jumpKey_ = k;
  }

  inline int getDropKey() const
  {
    return dropKey_;
  }

  inline void setDropKey(int k)
  {
    dropKey_ = k;
  }

  inline bool isFrozen() const
  {
    return frozen_;
  }

  inline void setFrozen(bool f)
  {
    frozen_ = f;
  }

  inline bool isHoldingLeft() const
  {
    return holdingLeft_;
  }

  inline void setHoldingLeft(bool f)
  {
    holdingLeft_ = f;
  }

  inline bool isHoldingRight() const
  {
    return holdingRight_;
  }

  inline void setHoldingRight(bool f)
  {
    holdingRight_ = f;
  }

private:

  int leftKey_ = GLFW_KEY_LEFT;
  int rightKey_ = GLFW_KEY_RIGHT;
  int jumpKey_ = GLFW_KEY_UP;
  int dropKey_ = GLFW_KEY_DOWN;

  bool frozen_ = false;
  bool holdingLeft_ = false;
  bool holdingRight_ = false;
};

#endif /* end of include guard: CONTROLLABLE_H_4E0B85B4 */
