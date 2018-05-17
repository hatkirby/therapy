#ifndef AUTOMATABLE_H_3D519131
#define AUTOMATABLE_H_3D519131

#include "component.h"
#include <sol.hpp>
#include <memory>
#include "entity_manager.h"

class RunnableComponent : public Component {
public:

  using id_type = EntityManager::id_type;

  /**
   * A Lua stack where the entity's script is running.
   *
   * NOTE: This object is called a thread, but there is no multi-threading going
   * on.
   *
   * @managed_by ScriptingSystem
   */
  std::unique_ptr<sol::thread> runner;

  /**
   * An entry point to the script running in the runner thread.
   *
   * @managed_by ScriptingSystem
   */
  std::unique_ptr<sol::coroutine> callable;

  /**
   * Whether or not this entity represents a behavior script. A behavior script
   * usually does not terminate on its own, and can be terminated at will by
   * another system, usually when the automatable entity leaves the active map.
   *
   * @managed_by ScriptingSystem
   */
  bool behavior = false;

  /**
   * If this is a behavior script, this is the ID of the automatable entity that
   * the behavior belongs to. This is required so that the ScriptingSystem can
   * notify the automatable entity if the behavior script terminates by itself,
   * and that it shouldn't attempt to terminate it.
   *
   * @managed_by ScriptingSystem
   */
  id_type actor;
};

#endif /* end of include guard: AUTOMATABLE_H_3D519131 */
