#ifndef PROTOTYPABLE_H_817F2205
#define PROTOTYPABLE_H_817F2205

#include "component.h"
#include "entity_manager.h"

class PrototypableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  size_t mapObjectIndex;

  std::string prototypeId;

  bool hasBehavior = false;
  bool runningBehavior = false;

  id_type behaviorScript;
};

#endif /* end of include guard: PROTOTYPABLE_H_817F2205 */
