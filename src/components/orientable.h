#ifndef ORIENTABLE_H_EDB6C4A1
#define ORIENTABLE_H_EDB6C4A1

#include "component.h"

class OrientableComponent : public Component {
public:

  enum class WalkState {
    still,
    left,
    right
  };

  enum class DropState {
    none,
    ready,
    active
  };

  inline bool isFacingRight() const
  {
    return facingRight_;
  }

  inline void setFacingRight(bool v)
  {
    facingRight_ = v;
  }

  inline WalkState getWalkState() const
  {
    return walkState_;
  }

  inline void setWalkState(WalkState v)
  {
    walkState_ = v;
  }

  inline bool isJumping() const
  {
    return jumping_;
  }

  inline void setJumping(bool v)
  {
    jumping_ = v;
  }

  inline DropState getDropState() const
  {
    return dropState_;
  }

  inline void setDropState(DropState v)
  {
    dropState_ = v;
  }

private:

  bool facingRight_ = false;
  WalkState walkState_ = WalkState::still;
  bool jumping_ = false;
  DropState dropState_ = DropState::none;
};

#endif /* end of include guard: ORIENTABLE_H_EDB6C4A1 */
