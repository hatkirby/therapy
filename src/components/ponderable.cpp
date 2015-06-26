#include "ponderable.h"

double PonderableComponent::getVelocityX() const
{
  return velocityX;
}

void PonderableComponent::setVelocityX(double v)
{
  velocityX = v;
}

double PonderableComponent::getVelocityY() const
{
  return velocityY;
}

void PonderableComponent::setVelocityY(double v)
{
  velocityY = v;
}

double PonderableComponent::getAccelX() const
{
  return accelX;
}

void PonderableComponent::setAccelX(double v)
{
  accelX = v;
}

double PonderableComponent::getAccelY() const
{
  return accelY;
}

void PonderableComponent::setAccelY(double v)
{
  accelY = v;
}
