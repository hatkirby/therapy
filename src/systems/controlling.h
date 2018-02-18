#ifndef CONTROLLING_H_80B1BB8D
#define CONTROLLING_H_80B1BB8D

#include "system.h"
#include <queue>

class ControllingSystem : public System {
public:

  ControllingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void input(int key, int action);

  void freeze(id_type entity);

  void unfreeze(id_type entity);

private:

  std::queue<std::pair<int,int>> actions_;
};

#endif /* end of include guard: CONTROLLING_H_80B1BB8D */
