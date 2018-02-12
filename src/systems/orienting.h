#ifndef ORIENTING_H_099F0C23
#define ORIENTING_H_099F0C23

#include "system.h"

class OrientingSystem : public System {
public:

  OrientingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void moveLeft(id_type entity);

  void moveRight(id_type entity);

  void stopWalking(id_type entity);

  void jump(id_type entity);

  void stopJumping(id_type entity);

  void land(id_type entity);

  void startFalling(id_type entity);

  void drop(id_type entity);

  void stopDropping(id_type entity);

};

#endif /* end of include guard: ORIENTING_H_099F0C23 */
