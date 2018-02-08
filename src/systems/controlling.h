#ifndef CONTROLLING_H_80B1BB8D
#define CONTROLLING_H_80B1BB8D

#include "system.h"
#include <queue>
#include "entity_manager.h"

class ControllingSystem : public System {
public:

  ControllingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);
  void input(int key, int action);

private:

  void walkLeft(id_type entity);
  void walkRight(id_type entity);
  void stopWalking(id_type entity);
  void jump(id_type entity);
  void stopJumping(id_type entity);
  void drop(id_type entity, bool start);

  std::queue<std::pair<int,int>> actions_;
};

#endif /* end of include guard: CONTROLLING_H_80B1BB8D */
