#ifndef PROTOTYPABLE_H_817F2205
#define PROTOTYPABLE_H_817F2205

#include "component.h"
#include "entity_manager.h"

class PrototypableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * The index of the object in the map definition.
   */
  size_t mapObjectIndex;

  /**
   * The name of the prototype that the object was spawned from.
   */
  std::string prototypeId;
};

#endif /* end of include guard: PROTOTYPABLE_H_817F2205 */
