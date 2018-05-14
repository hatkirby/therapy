#ifndef AUTOMATABLE_H_FACB42A5
#define AUTOMATABLE_H_FACB42A5

#include "component.h"
#include "entity_manager.h"

class AutomatableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * Controls what script will be run as this entity's behavior. It should refer
   * to a table in the global namespace of the script engine state, and that
   * table should contain a function called "Behavior".
   */
  std::string table;

  /**
   * Whether or not the behavior script is running.
   *
   * @managed_by ScriptingSystem
   */
  bool running = false;

  /**
   * The entity ID of the running script, if there is one.
   *
   * @managed_by ScriptingSystem
   */
  id_type script;
};

#endif /* end of include guard: AUTOMATABLE_H_FACB42A5 */
