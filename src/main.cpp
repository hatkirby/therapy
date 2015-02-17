#include <ctime>
#include <list>
#include "map.h"
#include "state.h"
#include "mapview.h"
#include "renderer.h"

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
  GLFWwindow* window = initRenderer();
  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);
  
  Map* m = new Map("../maps/bigmap.txt");
  Map* m2 = new Map("../maps/cozy.txt");
  
  m->setLeftMap(m2);
  m2->setRightMap(m);
  
  curGameState = new MapView(m, 100, 100);
  
  Texture* buffer = createTexture(GAME_WIDTH, GAME_HEIGHT);
  
  while (!(quit || glfwWindowShouldClose(window)))
  {
    // Tick!
    curGameState->tick();
    
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
