#ifndef AUTOMATING_H_E6E5D76E
#define AUTOMATING_H_E6E5D76E

#include "system.h"

class AutomatingSystem : public System {
public:

  AutomatingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void initPrototype(id_type prototype);

};

#endif /* end of include guard: AUTOMATING_H_E6E5D76E */