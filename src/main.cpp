#include "muxer.h"
#include "game.h"

int main()
{
  initMuxer();

  Game game;
  game.execute();

  destroyMuxer();

  return 0;
}
