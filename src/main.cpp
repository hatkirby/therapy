#include <ctime>
#include <list>
#include "map.h"
#include "renderer.h"

using namespace::std;

const int FRAMES_PER_SECOND = 60;
bool holding_left = false;
bool holding_right = false;
bool quit = false;
mob_t* player;

// Initialize jump physics
double jump_height = TILE_HEIGHT*3;
double jump_length = 0.25 * FRAMES_PER_SECOND;
double jump_velocity = -2 * jump_height / jump_length;
double jump_gravity = -1 * jump_velocity / jump_length;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS)
  {
    switch (key)
    {
      case GLFW_KEY_LEFT: holding_left = true; break;
      case GLFW_KEY_RIGHT: holding_right = true; break;
      case GLFW_KEY_UP: player->y_vel = jump_velocity; break;
      case GLFW_KEY_ESCAPE: quit = true; break;
    }
  } else if (action == GLFW_RELEASE)
  {
    switch (key)
    {
      case GLFW_KEY_LEFT: holding_left = false; break;
      case GLFW_KEY_RIGHT: holding_right = false; break;
    }
  }
}

int main()
{
  GLFWwindow* window = initRenderer();
  glfwSwapInterval(1);
  glfwSetKeyCallback(window, key_callback);
  
  Texture* buffer = createTexture(GAME_WIDTH, GAME_HEIGHT);
  
  // Initialize player data
  player = new mob_t();
  player->x = 100;
  player->y = 100;
  player->x_vel = 0;
  player->y_vel = 0;
  player->x_accel = 0;
  player->y_accel = jump_gravity;
  player->w = 10;
  player->h = 14;
  
  Map* map = new Map();

  Texture* tiles = loadTextureFromBMP("../res/tiles.bmp");
  
  while (!quit)
  {
    if (holding_left && player->x_vel >= 0)
    {
      player->x_vel = -2;
    } else if (holding_right && player->x_vel <= 0)
    {
      player->x_vel = 2;
    } else if (!holding_left && !holding_right) {
      player->x_vel = 0;
    }
    
    player->x_vel += player->x_accel;
    if (player->x_vel < -16) player->x_vel = -16;
    if (player->x_vel > 16) player->x_vel = 16;
    int playerx_next = player->x + player->x_vel;
    
    player->y_vel += player->y_accel;
    if (player->y_vel > 16) player->y_vel = 16; // Terminal velocity
    if (player->y_vel < -16) player->y_vel = -16;
    int playery_next = player->y + player->y_vel;
    
    map->check_collisions(player, playerx_next, playery_next);
    
    // Do rendering
    map->render(buffer);
    
    //Rectangle src_rect(96, 0, 8, 8);
    Rectangle dst_rect(player->x, player->y, player->w, player->h);
    
    //blitTexture(tiles, buffer, &src_rect, &dst_rect);
    fillTexture(buffer, &dst_rect, 255, 255, 255);
    
    renderScreen(buffer);
    
    glfwPollEvents();
  }
  
  delete map;
  delete player;
  
  destroyRenderer();
  
  return 0;
}
