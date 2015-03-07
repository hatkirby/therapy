#ifndef WORLD_H
#define WORLD_H

class World;

#include <list>
#include "renderer.h"
#include "entity.h"
#include <cstdio>

class World {
  public:
    World() {};
    ~World() {};
    void tick();
    void input(int key, int action);
    void render(Texture* buffer);
    void addEntity(std::shared_ptr<Entity> e);
    
    std::list<Collidable*> bodies;
    std::shared_ptr<Entity> player;

  private:
    std::list<std::shared_ptr<Entity>> entities;
};

#endif
