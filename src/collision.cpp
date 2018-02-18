#include "collision.h"

bool Collision::operator<(const Collision& other) const
{
  // Most important is the type of collision
  if (type_ != other.type_)
  {
    return (static_cast<int>(type_) > static_cast<int>(other.type_));
  }

  // Next, categorize the collisions arbitrarily based on direction
  if (dir_ != other.dir_)
  {
    return (static_cast<int>(dir_) < static_cast<int>(other.dir_));
  }

  // We want to process closer collisions first
  if (axis_ != other.axis_)
  {
    switch (dir_)
    {
      case Direction::left:
      case Direction::up:
      {
        return (axis_ < other.axis_);
      }

      case Direction::right:
      case Direction::down:
      {
        return (axis_ > other.axis_);
      }
    }
  }

  // Order the remaining attributes arbitrarily
  return std::tie(collider_, lower_, upper_) <
    std::tie(other.collider_, other.lower_, other.upper_);
}

bool Collision::isColliding(
  double x,
  double y,
  int w,
  int h) const
{
  int right = x + w;
  int bottom = y + h;

  switch (dir_)
  {
    case Direction::left:
    case Direction::right:
    {
      if (!((bottom > lower_) && (y < upper_)))
      {
        return false;
      }

      break;
    }

    case Direction::up:
    case Direction::down:
    {
      if (!((right > lower_) && (x < upper_)))
      {
        return false;
      }

      break;
    }
  }

  switch (dir_)
  {
    case Direction::left:
    {
      return (axis_ >= x);
    }

    case Direction::right:
    {
      return (axis_ <= right);
    }

    case Direction::up:
    {
      return (axis_ >= y);
    }

    case Direction::down:
    {
      return (axis_ <= bottom);
    }
  }
}
