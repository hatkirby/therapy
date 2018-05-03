#include <random>
#include "muxer.h"
#include "game.h"

int main()
{
  std::random_device randomDevice;
  std::mt19937 rng(randomDevice());

  initMuxer();

  Game game(rng);
  game.execute();

  destroyMuxer();

  return 0;
}
