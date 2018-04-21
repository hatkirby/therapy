#include "prototype.h"
#include "game.h"
#include "animation.h"
#include "components/animatable.h"
#include "components/transformable.h"
#include "components/ponderable.h"
#include "systems/pondering.h"
#include "systems/playing.h"

id_type Prototype::instantiate(
  Game& game,
  const Map::Object& object) const
{
  id_type entity = game.getEntityManager().emplaceEntity();

  AnimationSet entityGraphics(spritePath_.c_str(), w_, h_, 1);
  entityGraphics.emplaceAnimation("default", 0, 1, 1);

  game.getEntityManager().emplaceComponent<AnimatableComponent>(
    entity,
    std::move(entityGraphics),
    "default");

  game.getEntityManager().emplaceComponent<TransformableComponent>(
    entity,
    object.getX(),
    object.getY(),
    w_,
    h_);

  game.getSystemManager().getSystem<PonderingSystem>().initializeBody(
    entity,
    PonderableComponent::BodyType::vacuumed,
    PonderableComponent::ColliderType::event);

  auto& ponderable = game.getEntityManager().
    getComponent<PonderableComponent>(entity);

  switch (action_)
  {
    case Action::save:
    {
      ponderable.setEventCallback(PonderableComponent::ColliderType::player,
        [] (Game& game) {
          auto& playing = game.getSystemManager().getSystem<PlayingSystem>();

          playing.save();
        });

      break;
    }
  }

  return entity;
}
