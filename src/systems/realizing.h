#ifndef REALIZING_H_6853748C
#define REALIZING_H_6853748C

#include "system.h"

class RealizingSystem : public System {
public:

  RealizingSystem(Game& game) : System(game)
  {
  }

  /**
   * Creates the singleton realizable entity and initializes it with the
   * provided world definition.
   */
  id_type initSingleton(std::string filename);

  /**
   * Helper method that returns the entity ID of the (assumed) singleton entity
   * with a RealizableComponent. Throws an exception if the number of realizable
   * entities is not exactly one.
   */
  id_type getSingleton() const;

  /**
   * Loads the given map.
   */
  void loadMap(id_type mapEntity);

  /**
   * Treats the given entity as part of the active map.
   */
  void enterActiveMap(id_type entity);

  /**
   * Stops treating the given entity as part of the active map.
   */
  void leaveActiveMap(id_type entity);

};

#endif /* end of include guard: REALIZING_H_6853748C */
