#include "droppable.h"

void DroppableComponent::setDroppable(bool can)
{
  droppable = can;
}

bool DroppableComponent::isDroppable() const
{
  return droppable;
}
