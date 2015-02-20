#include "mapview.h"

// Initialize jump physics
double jump_height = TILE_HEIGHT*3;
double jump_length = 0.25 * FRAMES_PER_SECOND;
double jump_velocity = -2 * jump_height / jump_length;
double jump_gravity = -1 * jump_velocity / jump_length;

MapView::MapView(Map* first, int x, int y)
{
  // Initialize player data
  player = new mob_t();
  player->x = x;
  player->y = y;
  player->x_vel = 0;
  player->y_vel = 0;
  player->x_accel = 0;
  player->y_accel = jump_gravity;
  player->w = 10;
  player->h = 12;
  player->onGround = false;
  player->animFrame = 0;
  
  bg = createTexture(GAME_WIDTH, GAME_HEIGHT);
  chara = loadTextureFromBMP("../res/Starla.bmp");
  tiles = loadTextureFromBMP("../res/tiles2.bmp");
  
  loadMap(first);
}

MapView::~MapView()
{
  destroyTexture(bg);
  destroyTexture(chara);
  destroyTexture(tiles);
  
  delete player;
}

void MapView::loadMap(Map* m)
{
  curMap = m;
  
  left_collisions.clear();
  right_collisions.clear();
  up_collisions.clear();
  down_collisions.clear();
  
  add_collision(-6, 0, GAME_WIDTH, left, (m->getLeftMap() == NULL) ? 1 : 2);
  add_collision(GAME_WIDTH+6, 0, GAME_WIDTH, right, (m->getRightMap() == NULL) ? 1 : 2);
  
  fillTexture(bg, NULL, 0, 0, 0);
  
  const int* mapbuf = m->mapdata();
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    Rectangle dst(x*TILE_WIDTH, y*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    Rectangle src(mapbuf[i]%8*TILE_WIDTH, mapbuf[i]/8*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    
    if (mapbuf[i] > 0)
    {
      blitTexture(tiles, bg, &src, &dst);
    }    
    //blitTexture(tiles, bg, &src, &dst);
    
    if ((mapbuf[i] > 0) && (!((mapbuf[i] >= 5) && (mapbuf[i] <= 7))))
    {
      //if ((x != 0) && (mapbuf[i-1] != 'X'))
      {
        add_collision(x*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, right, 0);
      }
      
      //if ((x != 39) && (mapbuf[i+1] != 'X'))
      {
        add_collision((x+1)*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, left, 0);
      }
      
      //if ((y != 0) && (mapbuf[i-MAP_WIDTH] != 'X'))
      {
        add_collision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, down, 0);
      }
      
      //if ((y != 23) && (mapbuf[i+MAP_WIDTH] != 'X'))
      {
        add_collision((y+1)*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, up, 0);
      }
    } else if ((mapbuf[i] >= 5) && (mapbuf[i] <= 7))
    {
      add_collision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, down, 3);
    }
  }
  
  Texture* font = loadTextureFromBMP("../res/font.bmp");
  const char* map_name = m->title();
  int start_x = (40/2) - (strlen(map_name)/2);
  for (size_t i=0; i<strlen(map_name); i++)
  {
    Rectangle srcRect(map_name[i] % 16 * 8, map_name[i] / 16 * 8, 8, 8);
    Rectangle dstRect((start_x + i)*8, 24*8, 8, 8);
    blitTexture(font, bg, &srcRect, &dstRect);
  }
  
  destroyTexture(font);
}

void MapView::input(int key, int action)
{
  if (action == GLFW_PRESS)
  {
    switch (key)
    {
      case GLFW_KEY_LEFT:
        holding_left = true;
        break;
      case GLFW_KEY_RIGHT:
        holding_right = true;
        break;
      case GLFW_KEY_UP:
        if (player->onGround)
        {
          player->y_vel = jump_velocity;
          player->onGround = false;
        }
        break;
      case GLFW_KEY_DOWN:
        holding_down = true;
        break;
    }
  } else if (action == GLFW_RELEASE)
  {
    switch (key)
    {
      case GLFW_KEY_LEFT:
        holding_left = false;
        if (!holding_right) player->animFrame = 1;
        break;
      case GLFW_KEY_RIGHT:
        holding_right = false;
        if (!holding_left) player->animFrame = 0;
        break;
      case GLFW_KEY_DOWN:
        holding_down = false;
        break;
    }
  }
}

void MapView::tick()
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
  
  check_collisions(player, playerx_next, playery_next);
}

void MapView::render(Texture* tex)
{
  if (animFrame == 0)
  {
    if (holding_left)
    {
      if (player->animFrame == 3)
      {
        player->animFrame = 5;
      } else {
        player->animFrame = 3;
      }
    } else if (holding_right)
    {
      if (player->animFrame == 2)
      {
        player->animFrame = 4;
      } else {
        player->animFrame = 2;
      }
    }
  }
  
  animFrame++;
  animFrame %= 10;
  
  // Draw the background
  blitTexture(bg, tex, NULL, NULL);
  
  // Draw the player
  Rectangle src_rect(player->animFrame * 10, 0, 10, 12);
  Rectangle dst_rect(player->x, player->y, player->w, player->h);
  blitTexture(chara, tex, &src_rect, &dst_rect);
}

void MapView::add_collision(int axis, int lower, int upper, direction_t dir, int type)
{
  //printf("added collision\n");
  list<collision_t>::iterator it;
  
  switch (dir)
  {
    case up:
      it = up_collisions.begin();
      for (; it!=up_collisions.end(); it++)
      {
        if (it->axis < axis) break;
      }
    
      up_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case down:
      it = down_collisions.begin();
      for (; it!=down_collisions.end(); it++)
      {
        if (it->axis > axis) break;
      }
    
      down_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case left:
      it = left_collisions.begin();
      for (; it!=left_collisions.end(); it++)
      {
        if (it->axis < axis) break;
      }
  
      left_collisions.insert(it, {axis, lower, upper, type});
      
      break;
    case right:
      it = right_collisions.begin();
      for (; it!=right_collisions.end(); it++)
      {
        if (it->axis > axis) break;
      }
  
      right_collisions.insert(it, {axis, lower, upper, type});
      
      break;
  }
}

void MapView::check_collisions(mob_t* mob, int x_next, int y_next)
{
  if (x_next < mob->x)
  {
    for (list<collision_t>::iterator it=left_collisions.begin(); it!=left_collisions.end(); it++)
    {
      if (it->axis > mob->x) continue;
      if (it->axis < x_next) break;
  
      if ((mob->y+mob->h > it->lower) && (mob->y < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          x_next = it->axis;
          mob->x_vel = 0;
        } else if (it->type == 1)
        {
          x_next = GAME_WIDTH-mob->w/2;
        } else if (it->type == 2)
        {
          x_next = GAME_WIDTH-mob->w/2;
          loadMap(curMap->getLeftMap());
        }
    
        break;
      }
    }
  } else if (x_next > mob->x)
  {
    for (list<collision_t>::iterator it=right_collisions.begin(); it!=right_collisions.end(); it++)
    {
      if (it->axis < mob->x+mob->w) continue;
      if (it->axis > x_next+mob->w) break;
  
      if ((mob->y+mob->h > it->lower) && (mob->y < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          x_next = it->axis - mob->w;
          mob->x_vel = 0;
        } else if (it->type == 1)
        {
          x_next = -mob->w/2;
        } else if (it->type == 2)
        {
          x_next = -mob->w/2;
          loadMap(curMap->getRightMap());
        }
    
        break;
      }
    }
  }
  
  mob->x = x_next;
  
  if (y_next < mob->y)
  {
    for (list<collision_t>::iterator it=up_collisions.begin(); it!=up_collisions.end(); it++)
    {
      if (it->axis > mob->y) continue;
      if (it->axis < y_next) break;
  
      if ((mob->x+mob->w > it->lower) && (mob->x < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          y_next = it->axis;
          mob->y_vel = 0;
        } else if (it->type == 1)
        {
          y_next = GAME_HEIGHT-mob->h/2-1;
        }
    
        break;
      }
    }
  } else if (y_next > mob->y)
  {
    for (list<collision_t>::iterator it=down_collisions.begin(); it!=down_collisions.end(); it++)
    {
      if (it->axis < mob->y+mob->h) continue;
      if (it->axis > y_next+mob->h) break;

      if ((mob->x+mob->w > it->lower) && (mob->x < it->upper))
      {
        // We have a collision!
        if (it->type == 0)
        {
          y_next = it->axis - mob->h;
          mob->y_vel = 0;
          mob->onGround = true;
        } else if (it->type == 1)
        {
          y_next = 1 - mob->h/2;
        } else if (it->type == 3)
        {
          if (holding_down)
          {
            holding_down = false;
          } else {
            y_next = it->axis - mob->h;
            mob->y_vel = 0;
            mob->onGround = true;
          }
        }
        
        break;
      }
    }
  }
  
  mob->y = y_next;
}
