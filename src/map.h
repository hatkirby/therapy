#include <list>
#include <vector>
#include "mob.h"
#include "renderer.h"

using namespace::std;

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT;

enum direction_t {
  up, left, down, right
};

typedef struct {
  int axis;
  int lower;
  int upper;
  int type;
} collision_t;

class Map {
public:
  Map();
  ~Map();
  void render(Texture* buffer);
  void check_collisions(mob_t* mob, int x_next, int y_next);
  
private:
  void add_collision(int axis, int lower, int upper, direction_t dir, int type);
  
  list<collision_t> left_collisions;
  list<collision_t> right_collisions;
  list<collision_t> up_collisions;
  list<collision_t> down_collisions;
  
  Texture* bg;
};