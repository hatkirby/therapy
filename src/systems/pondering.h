#ifndef PONDERING_H_F2530E0E
#define PONDERING_H_F2530E0E

#include "system.h"

class PonderingSystem : public System {
  public:
    PonderingSystem(Game& game)
      : System(game) {}
    
    void tick(double dt);
};

#endif /* end of include guard: PONDERING_H_F2530E0E */
