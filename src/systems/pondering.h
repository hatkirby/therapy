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

  /**
   * Initializes a ponderable map object from its prototype data.
   *
   * @requires entity is ponderable
   * @requires entity is a map object
   */
  void initPrototype(id_type prototype);

  /**
   * Unferries an entity if it is a passenger of another entity. Use before
   * moving a ponderable entity outside the PonderingSystem.
   *
   * @requires entity is ponderable
   */
  void unferry(id_type entity);

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

  void tickBody(
    id_type entity,
    double dt,
    const std::set<id_type>& entities);

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
