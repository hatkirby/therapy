#include "scheduling.h"
#include "game.h"
#include "components/schedulable.h"
#include "util.h"

void SchedulingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    SchedulableComponent>();

  for (id_type entity : entities)
  {
    auto& schedulable = game_.getEntityManager().
      getComponent<SchedulableComponent>(entity);

    for (auto& action : schedulable.actions)
    {
      std::get<0>(action) -= dt;

      if (std::get<0>(action) < 0)
      {
        std::get<1>(action)(entity);
      }
    }

    erase_if(schedulable.actions,
      [] (const SchedulableComponent::Action& action) {
        return (std::get<0>(action) < 0);
      });

    if (schedulable.actions.empty())
    {
      game_.getEntityManager().removeComponent<SchedulableComponent>(entity);
    }
  }
}

void SchedulingSystem::schedule(
  id_type entity,
  double length,
  std::function<void(id_type)> action)
{
  if (!game_.getEntityManager().hasComponent<SchedulableComponent>(entity))
  {
    game_.getEntityManager().emplaceComponent<SchedulableComponent>(entity);
  }

  auto& schedulable = game_.getEntityManager().
    getComponent<SchedulableComponent>(entity);

  schedulable.actions.emplace_back(
    length,
    std::move(action));
}
