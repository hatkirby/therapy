#include "entity.h"

void Entity::addComponent(std::shared_ptr<Component> c)
{
  components.push_back(c);
}

void Entity::send(Game& game, Message& msg)
{
  for (auto component : components)
  {
    component->receive(game, *this, msg);
  }
}

void Entity::tick(Game& game)
{
  for (auto component : components)
  {
    component->tick(game, *this);
  }
}

void Entity::input(Game& game, int key, int action)
{
  for (auto component : components)
  {
    component->input(game, *this, key, action);
  }
}

void Entity::render(Game& game, Texture& buffer)
{
  for (auto component : components)
  {
    component->render(game, *this, buffer);
  }
}

void Entity::detectCollision(Game& game, Entity& collider, std::pair<double, double> old_position)
{
  for (auto component : components)
  {
    component->detectCollision(game, *this, collider, old_position);
  }
}
