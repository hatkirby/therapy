#include "map.h"

struct platform_t {
  int x;
  int y;
  int w;
  int h;
  bool enter_from_left;
  bool enter_from_right;
  bool enter_from_top;
  bool enter_from_bottom;
  int r;
  int g;
  int b;
  int a;
};

Map::Map()
{
  add_collision(-6, 0, GAME_WIDTH, left, 1);
  add_collision(GAME_WIDTH+6, 0, GAME_WIDTH, right, 1);
  
  FILE* f = fopen("../maps/bigmap.txt", "r");
  char* mapbuf = (char*) malloc(MAP_WIDTH*(MAP_HEIGHT-1)*sizeof(char));
  
  for (int i=0; i<MAP_HEIGHT-1; i++)
  {
    fread(mapbuf + i*MAP_WIDTH, sizeof(char), MAP_WIDTH, f);
    fgetc(f);
  }
  
  fclose(f);
  
  //Texture* tiles = loadTextureFromBMP("../res/tiles.bmp");
  bg = createTexture(GAME_WIDTH, GAME_HEIGHT);
  fillTexture(bg, NULL, 0, 0, 0);
  
  for (int i=0; i<MAP_WIDTH*(MAP_HEIGHT-1); i++)
  {
    int x = i % MAP_WIDTH;
    int y = i / MAP_WIDTH;
    Rectangle dst(x*TILE_WIDTH, y*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
    //Rectangle src;
    
    switch (mapbuf[i])
    {
      case ' ': break;
      case 'X': fillTexture(bg, &dst, 255, 85, 85); break;
      case 'P': fillTexture(bg, &dst, 85, 255, 255); break;
    }
    
    //blitTexture(tiles, bg, &src, &dst);
    
    if (mapbuf[i] == 'X')
    {
      if ((x != 0) && (mapbuf[i-1] != 'X'))
      {
        add_collision(x*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, right, 0);
      }
      
      if ((x != 15) && (mapbuf[i+1] != 'X'))
      {
        add_collision((x+1)*TILE_WIDTH, y*TILE_HEIGHT, (y+1)*TILE_HEIGHT, left, 0);
      }
      
      if ((y != 0) && (mapbuf[i-MAP_WIDTH] != 'X'))
      {
        add_collision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, down, 0);
      }
      
      if ((y != 15) && (mapbuf[i+MAP_WIDTH] != 'X'))
      {
        add_collision((y+1)*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, up, 0);
      }
    } else if (mapbuf[i] == 'P')
    {
      add_collision(y*TILE_HEIGHT, x*TILE_WIDTH, (x+1)*TILE_WIDTH, down, 0);
    }
  }
  
  Texture* font = loadTextureFromBMP("../res/font.bmp");
  const char* map_name = "Everything Is Embarassing";
  int start_x = (40/2) - (strlen(map_name)/2);
  for (int i=0; i<strlen(map_name); i++)
  {
    Rectangle srcRect(map_name[i] % 16 * 8, map_name[i] / 16 * 8, 8, 8);
    Rectangle dstRect((start_x + i)*8, 24*8, 8, 8);
    blitTexture(font, bg, &srcRect, &dstRect);
  }
  
  destroyTexture(font);
  //destroyTexture(tiles);
}

Map::~Map()
{
  destroyTexture(bg);
}

void Map::add_collision(int axis, int lower, int upper, direction_t dir, int type)
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

void Map::check_collisions(mob_t* mob, int x_next, int y_next)
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
        } else if (it->type == 1)
        {
          y_next = 1 - mob->h/2;
        }
        
        break;
      }
    }
  }
  
  mob->y = y_next;
}

void Map::render(Texture* buffer)
{
  blitTexture(bg, buffer, NULL, NULL);
}
