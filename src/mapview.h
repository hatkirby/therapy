#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <list>
#include "state.h"
#include "mob.h"
#include "map.h"

using namespace::std;

const int TILE_WIDTH = 8;
const int TILE_HEIGHT = 8;
const int GAME_WIDTH = 320;
const int GAME_HEIGHT = 200;
const int MAP_WIDTH = GAME_WIDTH/TILE_WIDTH;
const int MAP_HEIGHT = GAME_HEIGHT/TILE_HEIGHT;

const int FRAMES_PER_SECOND = 60;
const double SECONDS_PER_FRAME = 1.0 / FRAMES_PER_SECOND;

enum direction_t {
  up, left, down, right
};

typedef struct {
  int axis;
  int lower;
  int upper;
  int type;
} collision_t;

class MapView : public State {
  public:
    MapView(Map* start, int x, int y);
    ~MapView();
    void loadMap(Map* m);
    void input(int key, int action);
    void tick();
    void render(Texture* tex);
    
  private:
    void add_collision(int axis, int lower, int upper, direction_t dir, int type);
    void check_collisions(mob_t* mob, int x_next, int y_next);
  
    list<collision_t> left_collisions;
    list<collision_t> right_collisions;
    list<collision_t> up_collisions;
    list<collision_t> down_collisions;
  
    Texture* bg = NULL;
    Texture* chara;
    Texture* tiles;
    
    bool holding_left = false;
    bool holding_right = false;
    bool holding_down = false;
    bool holding_up = false;
    mob_t* player;
    
    Map* curMap;
    
    int animFrame = 0;
};

#endif
