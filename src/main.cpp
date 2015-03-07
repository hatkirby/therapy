#include <ctime>
#include <list>
#include "renderer.h"
#include <cstdlib>
#include "game.h"

int main()
{
  srand(time(NULL));
  
  Game::getInstance().execute();
  
  return 0;
}
