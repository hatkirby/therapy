#ifndef CONTROLLABLE_H_4E0B85B4
#define CONTROLLABLE_H_4E0B85B4

#include "component.h"
#include "renderer.h"

class ControllableComponent : public Component {
  public:
    int getLeftKey() const;
    void setLeftKey(int k);
    int getRightKey() const;
    void setRightKey(int k);
    int getJumpKey() const;
    void setJumpKey(int k);
    int getDropKey() const;
    void setDropKey(int k);
    
    bool isFrozen() const;
    void setFrozen(bool f);
    bool isHoldingLeft() const;
    void setHoldingLeft(bool f);
    bool isHoldingRight() const;
    void setHoldingRight(bool f);
    
  private:
    int leftKey = GLFW_KEY_LEFT;
    int rightKey = GLFW_KEY_RIGHT;
    int jumpKey = GLFW_KEY_UP;
    int dropKey = GLFW_KEY_DOWN;
    
    bool frozen = false;
    bool holdingLeft = false;
    bool holdingRight = false;
};

#endif /* end of include guard: CONTROLLABLE_H_4E0B85B4 */
