#ifndef AUTOMATING_H_E6E5D76E
#define AUTOMATING_H_E6E5D76E

#include "system.h"
#include <sol.hpp>

class ScriptingSystem : public System {
public:

  ScriptingSystem(Game& game);

  void tick(double dt);

  void killScript(id_type entity);

  id_type runBehaviorScript(id_type entity);

  void onTouch(id_type entity, id_type player);

private:

  template <typename... Args>
  id_type runScript(std::string event, id_type entity, Args&&... args);

  sol::state engine;
};

#endif /* end of include guard: AUTOMATING_H_E6E5D76E */
