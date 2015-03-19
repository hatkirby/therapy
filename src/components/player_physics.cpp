#include "player_physics.h"
#include "muxer.h"
#include "game.h"
#include "consts.h"

#define JUMP_VELOCITY(h, l) (-2 * (h) / (l))
#define JUMP_GRAVITY(h, l) (2 * ((h) / (l)) / (l))

PlayerPhysicsComponent::PlayerPhysicsComponent()
{
  jump_velocity = JUMP_VELOCITY(TILE_HEIGHT*4.5, 0.3);
  jump_gravity = JUMP_GRAVITY(TILE_HEIGHT*4.5, 0.3);
  jump_gravity_short = JUMP_GRAVITY(TILE_HEIGHT*3.5, 0.233);
  
  accel.second = jump_gravity_short;
}

void PlayerPhysicsComponent::receive(Game&, Entity& entity, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    velocity.first = -90;
    direction = -1;
  } else if (msg.type == Message::Type::walkRight)
  {
    velocity.first = 90;
    direction = 1;
  } else if (msg.type == Message::Type::stopWalking)
  {
    velocity.first = 0.0;
    direction = 0;
  } else if (msg.type == Message::Type::setHorizontalVelocity)
  {
    velocity.first = msg.velocity;
  } else if (msg.type == Message::Type::setVerticalVelocity)
  {
    velocity.second = msg.velocity;
  } else if (msg.type == Message::Type::hitTheGround)
  {
    if (isFalling)
    {
      playSound("res/Randomize27.wav", 0.05);
      isFalling = false;
    }
    
    velocity.second = 0.0;
  } else if (msg.type == Message::Type::jump)
  {
    playSound("res/Randomize87.wav", 0.25);
    
    velocity.second = jump_velocity;
    accel.second = jump_gravity;
  } else if (msg.type == Message::Type::stopJump)
  {
    accel.second = jump_gravity_short;
  } else if (msg.type == Message::Type::canDrop)
  {
    canDrop = true;
  } else if (msg.type == Message::Type::cantDrop)
  {
    canDrop = false;
  } else if (msg.type == Message::Type::drop)
  {
    if (canDrop)
    {
      canDrop = false;
    } else {
      entity.position.second = msg.dropAxis - entity.size.second;
      velocity.second = 0;
    }
  } else if (msg.type == Message::Type::die)
  {
    frozen = true;
  } else if (msg.type == Message::Type::stopDying)
  {
    frozen = false;
  }
}

void PlayerPhysicsComponent::tick(Game& game, Entity& entity, double dt)
{
  // If frozen, do nothing
  if (frozen)
  {
    return;
  }
  
  // Continue walking even if blocked earlier
  if (velocity.first == 0)
  {
    if (direction < 0)
    {
      velocity.first = -90;
    } else if (direction > 0)
    {
      velocity.first = 90;
    }
  }
  
  // Increase gravity at the height of jump
  if ((accel.second == jump_gravity) && (velocity.second >= 0))
  {
    accel.second = jump_gravity_short;
  }
  
  // Do the movement
  std::pair<double, double> old_position = entity.position;
  PhysicsBodyComponent::tick(game, entity, dt);
    
  // Check for collisions
  game.detectCollision(entity, old_position);
  
  // Are we moving due to gravity?
  if (velocity.second != 0.0)
  {
    isFalling = true;
  }
}
