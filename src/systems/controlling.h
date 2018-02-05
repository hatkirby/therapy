#ifndef CONTROLLING_H_80B1BB8D
#define CONTROLLING_H_80B1BB8D

#include "system.h"
#include <queue>

class ControllingSystem : public System {
  public:
    ControllingSystem(Game& game)
      : System(game) {}

    void tick(double dt);
    void input(int key, int action);

  private:
    void walkLeft(int entity);
    void walkRight(int entity);
    void stopWalking(int entity);
    void jump(int entity);
    void stopJumping(int entity);
    void drop(int entity, bool start);

    std::queue<std::pair<int,int>> actions;
};

#endif /* end of include guard: CONTROLLING_H_80B1BB8D */
