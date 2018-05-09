#include "automating.h"
#include "game.h"
#include "components/automatable.h"
#include "components/ponderable.h"
#include "systems/pondering.h"

void AutomatingSystem::tick(double dt)
{
  auto entities = game_.getEntityManager().getEntitiesWithComponents<
    AutomatableComponent,
    PonderableComponent>();

  for (id_type entity : entities)
  {
    auto& automatable = game_.getEntityManager().
      getComponent<AutomatableComponent>(entity);

    if (!automatable.active)
    {
      continue;
    }

    if (automatable.behaviorRunning &&
        (automatable.remaining <= 0.0))
    {
      automatable.currentAction++;
      automatable.actionRunning = false;

      if (automatable.currentAction ==
            automatable.behaviors[automatable.currentBehavior].size())
      {
        automatable.behaviorRunning = false;
      }
    }

    if (!automatable.behaviorRunning)
    {
      automatable.currentBehavior = automatable.behaviorDist(game_.getRng());
      automatable.currentAction = 0;
      automatable.behaviorRunning = true;
    }

    AutomatableComponent::Action& curAction =
      automatable.behaviors
        [automatable.currentBehavior]
          [automatable.currentAction];

    if (!automatable.actionRunning)
    {
      automatable.remaining = curAction.dur;
      automatable.actionRunning = true;
    }

    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(entity);

    ponderable.vel = curAction.speed;

    automatable.remaining -= dt;
  }
}

void AutomatingSystem::initPrototype(id_type prototype)
{
  auto& automatable = game_.getEntityManager().
    getComponent<AutomatableComponent>(prototype);

  automatable.behaviorRunning = false;
  automatable.actionRunning = false;
}
