#include "physics_body.h"
#include "game.h"
#include "consts.h"

void PhysicsBodyComponent::receive(Game&, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::walkLeft)
  {
    velocity.first = -90;
  } else if (msg.type == Message::Type::walkRight)
  {
    velocity.first = 90;
  } else if (msg.type == Message::Type::stopWalking)
  {
    velocity.first = 0.0;
  } else if (msg.type == Message::Type::setHorizontalVelocity)
  {
    velocity.first = msg.velocity;
  } else if (msg.type == Message::Type::setVerticalVelocity)
  {
    velocity.second = msg.velocity;
  }
}

void PhysicsBodyComponent::tick(Game&, Entity& entity, double dt)
{
  // Accelerate
  velocity.first += accel.first * dt;
  velocity.second += accel.second * dt;
  
  // Terminal velocity
#define TERMINAL_VELOCITY_X (2 * TILE_WIDTH * FRAMES_PER_SECOND)
#define TERMINAL_VELOCITY_Y (2 * TILE_HEIGHT * FRAMES_PER_SECOND)
  if (velocity.first < -TERMINAL_VELOCITY_X) velocity.first = -TERMINAL_VELOCITY_X;
  if (velocity.first > TERMINAL_VELOCITY_X) velocity.first = TERMINAL_VELOCITY_X;
  if (velocity.second < -TERMINAL_VELOCITY_Y) velocity.second = -TERMINAL_VELOCITY_Y;
  if (velocity.second > TERMINAL_VELOCITY_Y) velocity.second = TERMINAL_VELOCITY_Y;
  
  // Do the movement
  entity.position.first += velocity.first * dt;
  entity.position.second += velocity.second * dt;
}

void PhysicsBodyComponent::detectCollision(Game& game, Entity& entity, Entity& collider, std::pair<double, double> old_position)
{
  // If already colliding, do nothing!
  if ((old_position.first + collider.size.first > entity.position.first)
    && (old_position.first < entity.position.first + entity.size.first)
    && (old_position.second + collider.size.second > entity.position.second)
    && (old_position.second < entity.position.second + entity.size.second))
  {
    return;
  }
  
  // If newly colliding, SHOCK AND HORROR!
  if ((collider.position.first + collider.size.first > entity.position.first)
    && (collider.position.first < entity.position.first + entity.size.first)
    && (collider.position.second + collider.size.second > entity.position.second)
    && (collider.position.second < entity.position.second + entity.size.second))
  {
    Message msg(Message::Type::collision);
    msg.collisionEntity = &collider;
    
    entity.send(game, msg);
  }
}
