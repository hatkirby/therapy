#ifndef SYSTEM_H_B61A8CEA
#define SYSTEM_H_B61A8CEA

#include "entity_manager.h"

class Game;

class System {
public:

  using id_type = EntityManager::id_type;

  System(Game& game) : game_(game)
  {
  }

  virtual ~System() = default;

  virtual void tick(double dt) = 0;

protected:

  Game& game_;
};

#endif /* end of include guard: SYSTEM_H_B61A8CEA */
