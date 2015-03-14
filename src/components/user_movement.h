#ifndef USER_MOVEMENT_H
#define USER_MOVEMENT_H

#include "entity.h"

class Game;

class UserMovementComponent : public Component {
  public:
    void input(Game& game, Entity& entity, int key, int action);
    void receive(Game&, Entity&, const Message& msg);
      
  private:
    bool holdingLeft = false;
    bool holdingRight = false;
    bool frozen = false;
};

#endif
