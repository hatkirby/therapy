#ifndef PLAYING_H_70A54F7D
#define PLAYING_H_70A54F7D

#include "system.h"

class PlayingSystem : public System {
public:

  PlayingSystem(Game& game) : System(game)
  {
  }

  void initPlayer();

  void changeMap(
    id_type player,
    size_t mapId,
    double x,
    double y);

  void die(id_type player);

};

#endif /* end of include guard: PLAYING_H_70A54F7D */
