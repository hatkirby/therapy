#include <ctime>
#include <list>
#include <cstdlib>
#include "renderer.h"
#include "muxer.h"
#include "game.h"

int main()
{
  srand(time(NULL));
  
  GLFWwindow* window = initRenderer();
  glfwSwapInterval(1);
  
  initMuxer();
  
  // Put this in a block so game goes out of scope before we destroy the renderer
  {
    Game game {window};
    game.execute();
  }
  
  destroyMuxer();
  destroyRenderer();
  
  return 0;
}
