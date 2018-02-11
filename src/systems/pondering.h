#ifndef PONDERING_H_F2530E0E
#define PONDERING_H_F2530E0E

#include "system.h"
#include "components/mappable.h"
#include "components/ponderable.h"
#include "direction.h"

class PonderingSystem : public System {
public:

  PonderingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void initializeBody(id_type entity, PonderableComponent::Type type);

private:

  void processCollision(
    id_type entity,
    Direction dir,
    double& newX,
    double& newY,
    int axis,
    MappableComponent::Boundary::Type type);

};

#endif /* end of include guard: PONDERING_H_F2530E0E */
