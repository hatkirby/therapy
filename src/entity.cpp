#include "entity.h"

void Entity::addComponent(std::shared_ptr<Component> c)
{
  components.push_back(c);
}

void Entity::send(message_t msg)
{
  for (auto it = components.begin(); it != components.end(); it++)
  {
    (*it)->receive(msg);
  }
}

void Entity::tick()
{
  for (auto it = components.begin(); it != components.end(); it++)
  {
    (*it)->tick();
  }
}

void Entity::input(int key, int action)
{
  for (auto it = components.begin(); it != components.end(); it++)
  {
    (*it)->input(key, action);
  }
}

void Entity::render(Texture* buffer)
{
  for (auto it = components.begin(); it != components.end(); it++)
  {
    (*it)->render(buffer);
  }
}
