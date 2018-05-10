#include "automating.h"
#include "game.h"
#include "components/automatable.h"
#include "components/ponderable.h"
#include "components/realizable.h"
#include "components/transformable.h"
#include "systems/realizing.h"
#include "vector.h"

struct script_entity {
  using id_type = EntityManager::id_type;

  id_type id;

  script_entity(id_type id) : id(id)
  {
  }
};

void AutomatingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    AutomatableComponent>();

  for (id_type entity : entities)
  {
    auto& automatable = game_.getEntityManager().
      getComponent<AutomatableComponent>(entity);

    if (!automatable.active)
    {
      continue;
    }

    auto result = (*automatable.behavior)(dt);
    if (!result.valid())
    {
      sol::error e = result;
      throw std::runtime_error(e.what());
    }
  }
}

void AutomatingSystem::initPrototype(id_type prototype)
{
  auto& automatable = game_.getEntityManager().
    getComponent<AutomatableComponent>(prototype);

  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(
      game_.getSystemManager().getSystem<RealizingSystem>().getSingleton());
  automatable.behavior.reset();
  automatable.runner = std::unique_ptr<sol::thread>(new sol::thread(sol::thread::create(realizable.scriptEngine.lua_state())));
  automatable.behavior = std::unique_ptr<sol::coroutine>(new sol::coroutine(automatable.runner->state()["run"]));

  auto result = (*automatable.behavior)(script_entity(prototype));
  if (!result.valid())
  {
    sol::error e = result;
    throw std::runtime_error(e.what());
  }
}

void AutomatingSystem::initScriptEngine(sol::state& scriptEngine)
{
  scriptEngine.open_libraries(sol::lib::base, sol::lib::coroutine);
  scriptEngine.new_usertype<vec2d>(
    "vec2d",
    sol::constructors<vec2d(), vec2d(double, double)>(),
    "x", sol::property(
      [] (vec2d& v) -> double { return v.x(); },
      [] (vec2d& v, double x) { v.x() = x; }),
    "y", sol::property(
      [] (vec2d& v) -> double { return v.y(); },
      [] (vec2d& v, double y) { v.y() = y; }));

  scriptEngine.new_usertype<vec2i>(
    "vec2i",
    sol::constructors<vec2i(), vec2i(int, int)>(),
    "x", [] (vec2i& v) -> int& { return v.x(); },
    "y", [] (vec2i& v) -> int& { return v.y(); });

  scriptEngine.new_usertype<script_entity>(
    "entity",
    sol::constructors<script_entity(id_type)>(),
    "id", &script_entity::id,
    "transformable",
      [&] (script_entity& entity) -> TransformableComponent& {
        return game_.getEntityManager().
          getComponent<TransformableComponent>(entity.id);
      },
    "ponderable",
      [&] (script_entity& entity) -> PonderableComponent& {
        return game_.getEntityManager().
          getComponent<PonderableComponent>(entity.id);
      });

  scriptEngine.new_usertype<TransformableComponent>(
    "transformable",
    "pos", &TransformableComponent::pos);

  scriptEngine.new_usertype<PonderableComponent>(
    "ponderable",
    "vel", &PonderableComponent::vel,
    "accel", &PonderableComponent::accel);
}
