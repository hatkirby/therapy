#include "playing.h"
#include "game.h"
#include "components/transformable.h"
#include "components/animatable.h"
#include "components/playable.h"
#include "components/controllable.h"
#include "components/orientable.h"
#include "systems/mapping.h"
#include "systems/pondering.h"
#include "animation.h"

void PlayingSystem::tick(double)
{
  // Check if we need to change the map
  auto players = game_.getEntityManager().getEntitiesWithComponents<
    PlayableComponent>();

  for (id_type player : players)
  {
    auto& playable = game_.getEntityManager().
      getComponent<PlayableComponent>(player);

    if (playable.changingMap)
    {
      // Change the map!
      auto entities = game_.getEntityManager().getEntities();

      for (id_type entity : entities)
      {
        if (entity != player)
        {
          game_.getEntityManager().deleteEntity(entity);
        }
      }

      game_.getSystemManager().getSystem<MappingSystem>().
        loadMap(playable.newMapId);

      auto& transformable = game_.getEntityManager().
        getComponent<TransformableComponent>(player);

      transformable.setX(playable.newMapX);
      transformable.setY(playable.newMapY);

      playable.changingMap = false;

      break;
    }
  }
}

void PlayingSystem::initPlayer()
{
  id_type player = game_.getEntityManager().emplaceEntity();

  AnimationSet playerGraphics {"res/Starla.png", 10, 12, 6};
  playerGraphics.emplaceAnimation("stillLeft", 3, 1, 1);
  playerGraphics.emplaceAnimation("stillRight", 0, 1, 1);
  playerGraphics.emplaceAnimation("walkingLeft", 4, 2, 10);
  playerGraphics.emplaceAnimation("walkingRight", 1, 2, 10);

  game_.getEntityManager().emplaceComponent<AnimatableComponent>(
    player,
    std::move(playerGraphics),
    "stillLeft");

  game_.getEntityManager().emplaceComponent<TransformableComponent>(
    player,
    203, 44, 10, 12);

  game_.getSystemManager().getSystem<PonderingSystem>().initializeBody(
    player,
    PonderableComponent::Type::freefalling);

  game_.getEntityManager().emplaceComponent<ControllableComponent>(player);
  game_.getEntityManager().emplaceComponent<OrientableComponent>(player);
  game_.getEntityManager().emplaceComponent<PlayableComponent>(player);
}

void PlayingSystem::changeMap(
  size_t mapId,
  double x,
  double y)
{
  auto players = game_.getEntityManager().getEntitiesWithComponents<
    PlayableComponent>();

  for (id_type player : players)
  {
    auto& playable = game_.getEntityManager().
      getComponent<PlayableComponent>(player);

    playable.changingMap = true;
    playable.newMapId = mapId;
    playable.newMapX = x;
    playable.newMapY = y;
  }
}
