#include "scripting.h"
#include "game.h"
#include "components/runnable.h"
#include "components/ponderable.h"
#include "components/transformable.h"
#include "components/playable.h"
#include "components/mappable.h"
#include "components/prototypable.h"
#include "systems/realizing.h"
#include "vector.h"

struct script_entity {
  using id_type = EntityManager::id_type;

  id_type id;

  script_entity(id_type id) : id(id)
  {
  }
};

ScriptingSystem::ScriptingSystem(Game& game) : System(game)
{
  id_type entity = game_.getEntityManager().emplaceEntity();

  engine_.open_libraries(sol::lib::base, sol::lib::coroutine);

  engine_.new_usertype<vec2d>(
    "vec2d",
    sol::constructors<vec2d(), vec2d(double, double)>(),
    "x", sol::property(
      [] (vec2d& v) -> double { return v.x(); },
      [] (vec2d& v, double x) { v.x() = x; }),
    "y", sol::property(
      [] (vec2d& v) -> double { return v.y(); },
      [] (vec2d& v, double y) { v.y() = y; }));

  engine_.new_usertype<vec2i>(
    "vec2i",
    sol::constructors<vec2i(), vec2i(int, int)>(),
    "x", [] (vec2i& v) -> int& { return v.x(); },
    "y", [] (vec2i& v) -> int& { return v.y(); });

  engine_.new_usertype<script_entity>(
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
      },
    "mappable",
      [&] (script_entity& entity) -> MappableComponent& {
        return game_.getEntityManager().
          getComponent<MappableComponent>(entity.id);
      },
    "playable",
      [&] (script_entity& entity) -> PlayableComponent& {
        return game_.getEntityManager().
          getComponent<PlayableComponent>(entity.id);
      },
    "prototypable",
      [&] (script_entity& entity) -> PrototypableComponent& {
        return game_.getEntityManager().
          getComponent<PrototypableComponent>(entity.id);
      });

  engine_.new_usertype<TransformableComponent>(
    "transformable",
    "pos", &TransformableComponent::pos);

  engine_.new_usertype<PonderableComponent>(
    "ponderable",
    "vel", &PonderableComponent::vel,
    "accel", &PonderableComponent::accel);

  engine_.new_usertype<MappableComponent>(
    "mappable",
    "mapId", &MappableComponent::mapId);

  engine_.new_usertype<PlayableComponent>(
    "playable",
    "checkpointPos", &PlayableComponent::checkpointPos,
    "checkpointMapId", &PlayableComponent::checkpointMapId,
    "checkpointMapObject", &PlayableComponent::checkpointMapObject,
    "checkpointMapObjectIndex", &PlayableComponent::checkpointMapObjectIndex);

  engine_.new_usertype<PrototypableComponent>(
    "prototypable",
    "mapObjectIndex", &PrototypableComponent::mapObjectIndex,
    "prototypeId", &PrototypableComponent::prototypeId);

  engine_.new_usertype<RealizingSystem>(
    "realizing",
    "activeMap", sol::property(&RealizingSystem::getActiveMap));

  engine_.set_function(
    "realizing",
    [&] () {
      return game_.getSystemManager().getSystem<RealizingSystem>();
    });

  engine_.script_file("scripts/common.lua");
  engine_.script_file("scripts/movplat.lua");
  engine_.script_file("scripts/checkpoint.lua");
}

void ScriptingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    RunnableComponent>();

  for (id_type entity : entities)
  {
    auto& runnable = game_.getEntityManager().
      getComponent<RunnableComponent>(entity);

    if (*runnable.callable)
    {
      auto result = (*runnable.callable)(dt);
      if (!result.valid())
      {
        sol::error e = result;
        throw std::runtime_error(e.what());
      }
    }

    if (!*runnable.callable)
    {
      game_.getEntityManager().deleteEntity(entity);
    }
  }
}

void ScriptingSystem::killScript(id_type entity)
{
  if (game_.getEntityManager().hasComponent<RunnableComponent>(entity))
  {
    game_.getEntityManager().deleteEntity(entity);
  }
}

template <typename... Args>
EntityManager::id_type ScriptingSystem::runScript(
  std::string event,
  id_type entity,
  Args&&... args)
{
  auto& prototypable = game_.getEntityManager().
    getComponent<PrototypableComponent>(entity);

  id_type script = game_.getEntityManager().emplaceEntity();

  auto& runnable = game_.getEntityManager().
    emplaceComponent<RunnableComponent>(script);

  runnable.runner =
    std::unique_ptr<sol::thread>(
      new sol::thread(
        sol::thread::create(
          engine_.lua_state())));

  runnable.callable =
    std::unique_ptr<sol::coroutine>(
      new sol::coroutine(
        runnable.runner->state().
          traverse_get<sol::function>(
            prototypable.prototypeId,
            event)));

  if (!*runnable.callable)
  {
    throw std::runtime_error("Error running script");
  }

  auto result = (*runnable.callable)(
    script_entity(entity),
    std::forward<Args>(args)...);

  if (!result.valid())
  {
    sol::error e = result;
    throw std::runtime_error(e.what());
  }

  return script;
}

EntityManager::id_type ScriptingSystem::runBehaviorScript(id_type entity)
{
  return runScript("Behavior", entity);
}

void ScriptingSystem::onTouch(id_type entity, id_type player)
{
  runScript(
    "OnTouch",
    entity,
    script_entity(player));
}
