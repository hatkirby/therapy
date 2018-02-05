#include "transformable.h"

TransformableComponent::TransformableComponent(double x, double y, int w, int h)
  : x(x), y(y), w(w), h(h)
{

}

double TransformableComponent::getX() const
{
  return x;
}

double TransformableComponent::getY() const
{
  return y;
}

int TransformableComponent::getW() const
{
  return w;
}

int TransformableComponent::getH() const
{
  return h;
}

void TransformableComponent::setX(double v)
{
  x = v;
}

void TransformableComponent::setY(double v)
{
  y = v;
}

void TransformableComponent::setW(int v)
{
  w = v;
}

void TransformableComponent::setH(int v)
{
  h = v;
}
