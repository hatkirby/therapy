#include "playing.h"
#include "game.h"
#include "components/transformable.h"
#include "components/animatable.h"
#include "components/playable.h"
#include "components/controllable.h"
#include "components/orientable.h"
#include "components/mappable.h"
#include "systems/mapping.h"
#include "systems/pondering.h"
#include "systems/orienting.h"
#include "systems/scheduling.h"
#include "systems/controlling.h"
#include "animation.h"
#include "muxer.h"

void PlayingSystem::tick(double)
{
  // Check if we need to change the map
  auto players = game_.getEntityManager().getEntitiesWithComponents<
    PlayableComponent,
    TransformableComponent>();

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

      if (playable.newMapCallback)
      {
        playable.newMapCallback();
        playable.newMapCallback = nullptr;
      }

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
    game_.getWorld().getStartingX(),
    game_.getWorld().getStartingY(),
    10,
    12);

  game_.getSystemManager().getSystem<PonderingSystem>().initializeBody(
    player,
    PonderableComponent::BodyType::freefalling,
    PonderableComponent::ColliderType::player);

  game_.getEntityManager().emplaceComponent<ControllableComponent>(player);
  game_.getEntityManager().emplaceComponent<OrientableComponent>(player);

  auto& playable = game_.getEntityManager().
    emplaceComponent<PlayableComponent>(player);

  playable.checkpointMapId = game_.getWorld().getStartingMapId();
  playable.checkpointX = game_.getWorld().getStartingX();
  playable.checkpointY = game_.getWorld().getStartingY();
}

void PlayingSystem::changeMap(
  size_t mapId,
  double x,
  double y,
  PlayableComponent::MapChangeCallback callback)
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
    playable.newMapCallback = std::move(callback);
  }
}

void PlayingSystem::die()
{
  playSound("res/Hit_Hurt5.wav", 0.25);

  auto players = game_.getEntityManager().getEntitiesWithComponents<
    OrientableComponent,
    ControllableComponent,
    AnimatableComponent,
    PonderableComponent,
    PlayableComponent>();

  for (id_type player : players)
  {
    auto& animatable = game_.getEntityManager().
      getComponent<AnimatableComponent>(player);

    auto& ponderable = game_.getEntityManager().
      getComponent<PonderableComponent>(player);

    auto& controlling = game_.getSystemManager().getSystem<ControllingSystem>();
    controlling.freeze(player);

    animatable.setFrozen(true);
    animatable.setFlickering(true);
    ponderable.setFrozen(true);
    ponderable.setCollidable(false);

    auto& scheduling = game_.getSystemManager().getSystem<SchedulingSystem>();

    scheduling.schedule(player, 0.75, [&] (id_type player) {
      auto& playable = game_.getEntityManager().
        getComponent<PlayableComponent>(player);

      changeMap(
        playable.checkpointMapId,
        playable.checkpointX,
        playable.checkpointY,
        [&, player] () {
          animatable.setFrozen(false);
          animatable.setFlickering(false);
          ponderable.setFrozen(false);
          ponderable.setCollidable(true);

          // Reset the walk state, and then potentially let the
          // ControllingSystem set it again.
          auto& orienting = game_.getSystemManager().
            getSystem<OrientingSystem>();
          orienting.stopWalking(player);

          controlling.unfreeze(player);
        });
    });
  }
}

void PlayingSystem::save()
{
  playSound("res/Pickup_Coin23.wav", 0.25);

  auto players = game_.getEntityManager().getEntitiesWithComponents<
    TransformableComponent,
    PlayableComponent>();

  auto maps = game_.getEntityManager().getEntitiesWithComponents<
    MappableComponent>();

  auto& mappable = game_.getEntityManager().
    getComponent<MappableComponent>(*maps.begin());

  for (id_type player : players)
  {
    auto& transformable = game_.getEntityManager().
      getComponent<TransformableComponent>(player);

    auto& playable = game_.getEntityManager().
      getComponent<PlayableComponent>(player);

    playable.checkpointMapId = mappable.getMapId();
    playable.checkpointX = transformable.getX();
    playable.checkpointY = transformable.getY();
    playable.checkpointObjectActivated = false;
  }
}
