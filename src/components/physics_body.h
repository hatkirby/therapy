#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include "entity.h"
#include <utility>

class Game;

class PhysicsBodyComponent : public Component {
  public:
    void receive(Game& game, Entity& entity, const Message& msg);
    void tick(Game& game, Entity& entity, double dt);
    void detectCollision(Game& game, Entity& entity, Entity& collider, std::pair<double, double> old_position);
    
  protected:    
    std::pair<double, double> velocity;
    std::pair<double, double> accel;
};

#endif
