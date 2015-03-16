#include "player_sprite.h"

PlayerSpriteComponent::PlayerSpriteComponent() : sprite("res/Starla.png")
{
  
}

void PlayerSpriteComponent::render(Game&, Entity& entity, Texture& buffer)
{
  animFrame++;
  
  int frame = 0;
  if (isMoving)
  {
    frame += 2;
    
    if (animFrame % 20 < 10)
    {
      frame += 2;
    }
  }
  
  if (facingLeft)
  {
    frame++;
  }
  
  double alpha = 1.0;
  if (dying && (animFrame % 4 < 2))
  {
    alpha = 0.0;
  }
  
  Rectangle src_rect {frame*10, 0, 10, 12};
  Rectangle dst_rect {(int) entity.position.first, (int) entity.position.second, entity.size.first, entity.size.second};
  buffer.blit(sprite, src_rect, dst_rect, alpha);
}

void PlayerSpriteComponent::receive(Game&, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    facingLeft = true;
    isMoving = true;
  } else if (msg.type == Message::Type::walkRight)
  {
    facingLeft = false;
    isMoving = true;
  } else if (msg.type == Message::Type::stopWalking)
  {
    isMoving = false;
  } else if (msg.type == Message::Type::die)
  {
    dying = true;
    isMoving = false;
  } else if (msg.type == Message::Type::stopDying)
  {
    dying = false;
  }
}
