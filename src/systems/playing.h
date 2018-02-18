#ifndef PLAYING_H_70A54F7D
#define PLAYING_H_70A54F7D

#include "system.h"
#include "components/playable.h"

class PlayingSystem : public System {
public:

  PlayingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

  void initPlayer();

  void changeMap(
    size_t mapId,
    double x,
    double y,
    PlayableComponent::MapChangeCallback callback = nullptr);

  void die();

};

#endif /* end of include guard: PLAYING_H_70A54F7D */
