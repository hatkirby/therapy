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

  void initializeBody(id_type entity, PonderableComponent::Type type);

  void initPrototype(id_type prototype);

private:

  struct CollisionResult
  {
    double newX;
    double newY;
    bool stopProcessing = false;
    bool touchedWall = false;
    bool adjacentlyWarping = false;
    Direction adjWarpDir;
    size_t adjWarpMapId;
    id_type groundEntity;
  };

  void processCollision(
    id_type entity,
    id_type collider,
    Direction dir,
    PonderableComponent::Collision type,
    double axis,
    double lower,
    double upper,
    CollisionResult& result);

};

#endif /* end of include guard: PONDERING_H_F2530E0E */
