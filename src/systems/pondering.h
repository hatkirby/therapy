#ifndef PONDERING_H_F2530E0E
#define PONDERING_H_F2530E0E

#include "system.h"
#include "components/ponderable.h"
#include "direction.h"

class PonderingSystem : public System {
public:

  PonderingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void initializeBody(
    id_type entity,
    PonderableComponent::BodyType bodyType,
    PonderableComponent::ColliderType colliderType);

private:

  void processBodyCollision(id_type body, id_type collider);

};

#endif /* end of include guard: PONDERING_H_F2530E0E */
