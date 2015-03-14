#ifndef PLAYER_PHYSICS_H
#define PLAYER_PHYSICS_H

#include "entity.h"
#include "physics_body.h"

class Game;

class PlayerPhysicsComponent : public PhysicsBodyComponent {
  public:
    PlayerPhysicsComponent();
    void tick(Game& game, Entity& entity, double dt);
    void receive(Game& game, Entity& entity, const Message& msg);
    
  private:
    double jump_velocity;
    double jump_gravity;
    double jump_gravity_short;
    int direction = 0;
    bool canDrop = false;
    bool frozen = false;
    bool isFalling = false;
};

#endif
