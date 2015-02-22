#include <ctime>
#include <list>
#include "map.h"
#include "state.h"
#include "mapview.h"
#include "renderer.h"
#include <cstdlib>

using namespace::std;

bool quit = false;

State* curGameState;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
  {
    quit = true;
  }
  
  if (curGameState != NULL)
  {
    curGameState->input(key, action);
  }
}

int main()
{
  srand(time(NULL));
  
  GLFWwindow* window = initRenderer();
  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);
  
  Map* m = new Map("../maps/embarass.txt");
  //Map* m2 = new Map("../maps/cozy.txt");
  
  //m->setLeftMap(m2);
  //m2->setRightMap(m);
  
  curGameState = new MapView(m, 100, 100);
  
  Texture* buffer = createTexture(GAME_WIDTH, GAME_HEIGHT);
  
  double lastTime = glfwGetTime();
  double accum = 0.0;
  
  while (!(quit || glfwWindowShouldClose(window)))
  {
    // Tick!
    accum += (glfwGetTime() - lastTime);
    if (accum < 0) accum = 0;
    while (accum > SECONDS_PER_FRAME)
    {
      curGameState->tick();
      accum -= SECONDS_PER_FRAME;
    }
    
    lastTime = glfwGetTime();
    
    // Do rendering
    curGameState->render(buffer);
    renderScreen(buffer);
    
    glfwPollEvents();
  }
  
  delete curGameState;
  delete m;
  
  destroyRenderer();
  
  return 0;
}
