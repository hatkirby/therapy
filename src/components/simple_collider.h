#ifndef SIMPLE_COLLIDER_H
#define SIMPLE_COLLIDER_H

#include "entity.h"
#include <functional>

class Game;

class SimpleColliderComponent : public Component {
  public:
    SimpleColliderComponent(std::function<void (Game& game, Entity& collider)> callback);
    void receive(Game& game, Entity& entity, const Message& msg);
    
  private:
    std::function<void (Game& game, Entity& collider)> callback;
};

#endif
