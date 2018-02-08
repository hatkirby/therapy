#ifndef RENDERING_H_76ABC02A
#define RENDERING_H_76ABC02A

#include "system.h"
#include "renderer.h"
#include "consts.h"

class RenderingSystem : public System {
public:

  RenderingSystem(Game& game) : System(game)
  {
  }

  void tick(double dt);

private:

  Texture texture_ {GAME_WIDTH, GAME_HEIGHT};
};

#endif /* end of include guard: RENDERING_H_76ABC02A */
