#include "playing.h"
#include "game.h"
#include "components/transformable.h"
#include "components/animatable.h"
#include "components/playable.h"
#include "components/controllable.h"
#include "components/orientable.h"
#include "components/realizable.h"
#include "systems/mapping.h"
#include "systems/pondering.h"
#include "systems/orienting.h"
#include "systems/scheduling.h"
#include "systems/controlling.h"
#include "systems/animating.h"
#include "systems/realizing.h"
#include "animation.h"
#include "muxer.h"

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
    std::move(playerGraphics));

  game_.getSystemManager().getSystem<AnimatingSystem>().startAnimation(
    player,
    "stillLeft");

  auto& realizing = game_.getSystemManager().getSystem<RealizingSystem>();

  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(realizing.getSingleton());

  auto& transformable = game_.getEntityManager().
    emplaceComponent<TransformableComponent>(player);

  transformable.pos = realizable.startingPos;
  transformable.size.w() = 10;
  transformable.size.h() = 12;

  game_.getSystemManager().getSystem<PonderingSystem>().initializeBody(
    player,
    PonderableComponent::Type::freefalling);

  game_.getEntityManager().emplaceComponent<ControllableComponent>(player);
  game_.getEntityManager().emplaceComponent<OrientableComponent>(player);

  auto& playable = game_.getEntityManager().
    emplaceComponent<PlayableComponent>(player);

  playable.mapId = realizable.activeMap;
  playable.checkpointMapId = realizable.startingMapId;
  playable.checkpointPos = realizable.startingPos;

  realizing.enterActiveMap(player);

  realizable.activePlayer = player;
}

void PlayingSystem::changeMap(
  id_type player,
  size_t mapId,
  vec2d warpPos)
{
  auto& playable = game_.getEntityManager().
    getComponent<PlayableComponent>(player);

  auto& transformable = game_.getEntityManager().
    getComponent<TransformableComponent>(player);

  auto& pondering = game_.getSystemManager().getSystem<PonderingSystem>();

  auto& realizing = game_.getSystemManager().getSystem<RealizingSystem>();

  auto& realizable = game_.getEntityManager().
    getComponent<RealizableComponent>(realizing.getSingleton());

  id_type newMapEntity = realizable.entityByMapId[mapId];

  if (playable.mapId != newMapEntity)
  {
    if (playable.mapId == realizable.activeMap)
    {
      realizing.leaveActiveMap(player);
    } else if (newMapEntity == realizable.activeMap)
    {
      realizing.enterActiveMap(player);
    }

    playable.mapId = newMapEntity;
  }

  pondering.unferry(player);

  transformable.pos = warpPos;

  if (realizable.activePlayer == player)
  {
    realizing.loadMap(newMapEntity);
  }
}

void PlayingSystem::die(id_type player)
{
  playSound("res/Hit_Hurt5.wav", 0.25);

  auto& animatable = game_.getEntityManager().
    getComponent<AnimatableComponent>(player);

  auto& ponderable = game_.getEntityManager().
    getComponent<PonderableComponent>(player);

  auto& controlling = game_.getSystemManager().getSystem<ControllingSystem>();
  controlling.freeze(player);

  animatable.frozen = true;
  animatable.flickering = true;
  ponderable.frozen = true;
  ponderable.collidable = false;

  auto& scheduling = game_.getSystemManager().getSystem<SchedulingSystem>();

  scheduling.schedule(player, 0.75, [&] (id_type player) {
    auto& playable = game_.getEntityManager().
      getComponent<PlayableComponent>(player);

    changeMap(
      player,
      playable.checkpointMapId,
      playable.checkpointPos);

    animatable.frozen = false;
    animatable.flickering = false;
    ponderable.frozen = false;
    ponderable.collidable = true;

    // Reset the walk state, and then potentially let the
    // ControllingSystem set it again.
    auto& orienting = game_.getSystemManager().getSystem<OrientingSystem>();
    orienting.stopWalking(player);

    controlling.unfreeze(player);
  });
}
