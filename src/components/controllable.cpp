#include "controllable.h"

int ControllableComponent::getLeftKey() const
{
  return leftKey;
}

void ControllableComponent::setLeftKey(int k)
{
  leftKey = k;
}

int ControllableComponent::getRightKey() const
{
  return rightKey;
}

void ControllableComponent::setRightKey(int k)
{
  rightKey = k;
}

int ControllableComponent::getJumpKey() const
{
  return jumpKey;
}

void ControllableComponent::setJumpKey(int k)
{
  jumpKey = k;
}

int ControllableComponent::getDropKey() const
{
  return dropKey;
}

void ControllableComponent::setDropKey(int k)
{
  dropKey = k;
}

bool ControllableComponent::isFrozen() const
{
  return frozen;
}

void ControllableComponent::setFrozen(bool f)
{
  frozen = f;
}

bool ControllableComponent::isHoldingLeft() const
{
  return holdingLeft;
}

void ControllableComponent::setHoldingLeft(bool f)
{
  holdingLeft = f;
}

bool ControllableComponent::isHoldingRight() const
{
  return holdingRight;
}

void ControllableComponent::setHoldingRight(bool f)
{
  holdingRight = f;
}
