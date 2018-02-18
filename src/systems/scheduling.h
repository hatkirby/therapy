#ifndef SCHEDULING_H_7B02E3E3
#define SCHEDULING_H_7B02E3E3

#include "system.h"

class SchedulingSystem : public System {
public:

  SchedulingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void schedule(
    id_type entity,
    double length,
    std::function<void(id_type)> action);

};

#endif /* end of include guard: SCHEDULING_H_7B02E3E3 */
