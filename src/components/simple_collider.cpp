#include "simple_collider.h"

SimpleColliderComponent::SimpleColliderComponent(std::function<void (Game& game, Entity& collider)> callback) : callback(callback)
{
  
}

void SimpleColliderComponent::receive(Game& game, Entity&, const Message& msg)
{
  if (msg.type == Message::Type::collision)
  {
    callback(game, *msg.collisionEntity);
  }
}
