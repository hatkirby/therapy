#include "world.h"

void World::tick()
{
  for (auto it = entities.begin(); it != entities.end(); it++)
  {
    (*it)->tick();
  }
}

void World::input(int key, int action)
{
  for (auto it = entities.begin(); it != entities.end(); it++)
  {
    (*it)->input(key, action);
  }
}

void World::render(Texture* buffer)
{
  fillTexture(buffer, NULL, 0, 0, 0);
  
  for (auto it = entities.begin(); it != entities.end(); it++)
  {
    (*it)->render(buffer);
  }
}

void World::addEntity(std::shared_ptr<Entity> e)
{
  entities.push_back(e);
}
